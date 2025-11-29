from rest_framework import viewsets, status
from rest_framework.decorators import action
from rest_framework.response import Response
from django.shortcuts import get_object_or_404
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