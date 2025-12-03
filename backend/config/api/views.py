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
        """Abrir válvula del dispositivo"""
        device = self.get_object()
        
        if not device.is_online:
            return Response(
                {'error': 'Dispositivo no conectado'},
                status=status.HTTP_503_SERVICE_UNAVAILABLE
            )

        try:
            # Enviar comando al ESP32
            url = f"http://{device.ip_address}/api/valve/open"
            response = requests.post(url, timeout=5)
            
            if response.status_code == 200:
                # Registrar en base de datos
                ValveControl.objects.create(
                    device=device,
                    status='open'
                )
                return Response({'message': 'Válvula abierta'})
            else:
                return Response(
                    {'error': 'Error en ESP32'},
                    status=status.HTTP_500_INTERNAL_SERVER_ERROR
                )
        except requests.exceptions.RequestException as e:
            return Response(
                {'error': f'No se pudo conectar: {str(e)}'},
                status=status.HTTP_503_SERVICE_UNAVAILABLE
            )

    @action(detail=True, methods=['post'])
    def close_valve(self, request, pk=None):
        """Cerrar válvula del dispositivo"""
        device = self.get_object()
        
        if not device.is_online:
            return Response(
                {'error': 'Dispositivo no conectado'},
                status=status.HTTP_503_SERVICE_UNAVAILABLE
            )

        try:
            # Enviar comando al ESP32
            url = f"http://{device.ip_address}/api/valve/close"
            response = requests.post(url, timeout=5)
            
            if response.status_code == 200:
                # Registrar en base de datos
                ValveControl.objects.create(
                    device=device,
                    status='closed'
                )
                return Response({'message': 'Válvula cerrada'})
            else:
                return Response(
                    {'error': 'Error en ESP32'},
                    status=status.HTTP_500_INTERNAL_SERVER_ERROR
                )
        except requests.exceptions.RequestException as e:
            return Response(
                {'error': f'No se pudo conectar: {str(e)}'},
                status=status.HTTP_503_SERVICE_UNAVAILABLE
            )

    @action(detail=True, methods=['get'])
    def status(self, request, pk=None):
        """Obtener estado del dispositivo"""
        device = self.get_object()
        
        try:
            url = f"http://{device.ip_address}/api/status"
            response = requests.get(url, timeout=5)
            
            if response.status_code == 200:
                data = response.json()
                return Response(data)
        except:
            pass
        
        return Response({
            'id': device.id,
            'name': device.name,
            'is_online': device.is_online
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