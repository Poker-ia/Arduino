from rest_framework import viewsets, status
from rest_framework.decorators import action
from rest_framework.response import Response
from django.shortcuts import get_object_or_404
from django.db.models import Avg, Max, Min, Count
import requests
import json
from .models import Device, ValveControl, SensorReading
from .serializers import DeviceSerializer, ValveControlSerializer, SensorReadingSerializer


class DeviceViewSet(viewsets.ModelViewSet):
    queryset = Device.objects.all()
    serializer_class = DeviceSerializer

    @action(detail=True, methods=['post'])
    def open_valve(self, request, pk=None):
        """Solicitar apertura de válvula (arquitectura inversa)"""
        device = self.get_object()
        
        if not device.is_online:
            return Response(
                {'error': 'Dispositivo no conectado'},
                status=status.HTTP_503_SERVICE_UNAVAILABLE
            )

        # Guardar comando pendiente en base de datos
        device.desired_valve_state = 'open'
        device.save()
        
        return Response({
            'message': 'Comando de apertura registrado. El ESP32 lo ejecutará en breve.',
            'device_id': device.device_id,
            'desired_state': 'open'
        })

    @action(detail=True, methods=['post'])
    def close_valve(self, request, pk=None):
        """Solicitar cierre de válvula (arquitectura inversa)"""
        device = self.get_object()
        
        if not device.is_online:
            return Response(
                {'error': 'Dispositivo no conectado'},
                status=status.HTTP_503_SERVICE_UNAVAILABLE
            )

        # Guardar comando pendiente en base de datos
        device.desired_valve_state = 'closed'
        device.save()
        
        return Response({
            'message': 'Comando de cierre registrado. El ESP32 lo ejecutará en breve.',
            'device_id': device.device_id,
            'desired_state': 'closed'
        })
    
    @action(detail=False, methods=['get'])
    def get_pending_command(self, request):
        """ESP32 consulta comandos pendientes (polling)"""
        device_id = request.query_params.get('device_id')
        if not device_id:
            return Response(
                {'error': 'device_id es requerido'},
                status=status.HTTP_400_BAD_REQUEST
            )
        
        try:
            device = Device.objects.get(device_id=device_id)
            command = device.desired_valve_state
            
            # Si hay comando pendiente, limpiarlo después de enviarlo
            if command != 'none':
                device.desired_valve_state = 'none'
                device.save()
                
            return Response({
                'command': command,
                'device_id': device_id,
                'timestamp': device.updated_at
            })
        except Device.DoesNotExist:
            return Response(
                {'error': 'Dispositivo no encontrado'},
                status=status.HTTP_404_NOT_FOUND
            )
    
    @action(detail=False, methods=['post'])
    def report_valve_state(self, request):
        """ESP32 reporta el estado actual de la válvula"""
        device_id = request.data.get('device_id')
        valve_state = request.data.get('valve_state')
        
        if not device_id or not valve_state:
            return Response(
                {'error': 'device_id y valve_state son requeridos'},
                status=status.HTTP_400_BAD_REQUEST
            )
        
        try:
            device = Device.objects.get(device_id=device_id)
            device.current_valve_state = valve_state
            device.is_online = True  # Marcar como online al recibir reporte
            device.save()
            
            # Registrar en historial
            ValveControl.objects.create(
                device=device,
                status=valve_state
            )
            
            return Response({
                'message': 'Estado de válvula actualizado',
                'current_state': valve_state
            })
        except Device.DoesNotExist:
            return Response(
                {'error': 'Dispositivo no encontrado'},
                status=status.HTTP_404_NOT_FOUND
            )

    @action(detail=True, methods=['get'])
    def status(self, request, pk=None):
        """Obtener estado del dispositivo"""
        device = self.get_object()
        
        # Obtener última lectura del sensor
        latest_reading = SensorReading.objects.filter(device=device).first()
        
        return Response({
            'id': device.id,
            'name': device.name,
            'device_id': device.device_id,
            'is_online': device.is_online,
            'current_valve_state': device.current_valve_state,
            'desired_valve_state': device.desired_valve_state,
            'flow_rate': latest_reading.flow_rate if latest_reading else 0,
            'total_volume': latest_reading.total_volume if latest_reading else 0
        })


class ValveControlViewSet(viewsets.ModelViewSet):
    queryset = ValveControl.objects.all()
    serializer_class = ValveControlSerializer

    @action(detail=False, methods=['get'])
    def by_device(self, request):
        """Obtener historial de válvula por dispositivo"""
        device_id = request.query_params.get('device_id')
        if not device_id:
            return Response(
                {'error': 'device_id es requerido'},
                status=status.HTTP_400_BAD_REQUEST
            )
        
        controls = ValveControl.objects.filter(device_id=device_id)
        serializer = self.get_serializer(controls, many=True)
        return Response(serializer.data)


class SensorReadingViewSet(viewsets.ModelViewSet):
    queryset = SensorReading.objects.all()
    serializer_class = SensorReadingSerializer

    def create(self, request, *args, **kwargs):
        """Crear nueva lectura del sensor (usado por ESP32)"""
        # Permite enviar device_id en lugar de device
        if 'device_id' in request.data and 'device' not in request.data:
            try:
                device = Device.objects.get(device_id=request.data['device_id'])
                # Marcar dispositivo como online al recibir datos
                device.is_online = True
                device.save()
                
                # Crear copia mutable de request.data
                data = request.data.copy()
                data['device'] = device.id
                # Usar el serializer directamente con los datos modificados
                serializer = self.get_serializer(data=data)
                serializer.is_valid(raise_exception=True)
                self.perform_create(serializer)
                headers = self.get_success_headers(serializer.data)
                return Response(serializer.data, status=status.HTTP_201_CREATED, headers=headers)
            except Device.DoesNotExist:
                return Response(
                    {'error': f'Dispositivo con device_id={request.data["device_id"]} no encontrado'},
                    status=status.HTTP_404_NOT_FOUND
                )
        return super().create(request, *args, **kwargs)

    @action(detail=False, methods=['get'])
    def latest(self, request):
        """Obtener última lectura del sensor"""
        device_id = request.query_params.get('device_id')
        if not device_id:
            return Response(
                {'error': 'device_id es requerido'},
                status=status.HTTP_400_BAD_REQUEST
            )
        
        reading = SensorReading.objects.filter(device_id=device_id).first()
        if reading:
            serializer = self.get_serializer(reading)
            return Response(serializer.data)
        return Response({'error': 'Sin lecturas'}, status=status.HTTP_404_NOT_FOUND)

    @action(detail=False, methods=['get'])
    def stats(self, request):
        """Obtener estadísticas de consumo"""
        device_id = request.query_params.get('device_id')
        if not device_id:
            return Response(
                {'error': 'device_id es requerido'},
                status=status.HTTP_400_BAD_REQUEST
            )
        
        # Filtrar por rango de fechas si se proporciona
        start_date = request.query_params.get('start_date')
        end_date = request.query_params.get('end_date')
        
        readings = SensorReading.objects.filter(device_id=device_id)
        
        if start_date:
            readings = readings.filter(timestamp__gte=start_date)
        if end_date:
            readings = readings.filter(timestamp__lte=end_date)
        
        stats = readings.aggregate(
            avg_flow_rate=Avg('flow_rate'),
            max_flow_rate=Max('flow_rate'),
            min_flow_rate=Min('flow_rate'),
            total_readings=Count('id'),
            max_volume=Max('total_volume')
        )
        
        # Obtener última lectura para volumen actual
        latest = readings.first()
        
        return Response({
            'device_id': device_id,
            'current_volume': latest.total_volume if latest else 0,
            'avg_flow_rate': round(stats['avg_flow_rate'] or 0, 2),
            'max_flow_rate': round(stats['max_flow_rate'] or 0, 2),
            'min_flow_rate': round(stats['min_flow_rate'] or 0, 2),
            'total_readings': stats['total_readings'],
            'period_start': start_date,
            'period_end': end_date
        })