from rest_framework import serializers
from .models import Device, ValveControl, SensorReading

class DeviceSerializer(serializers.ModelSerializer):
    class Meta:
        model = Device
        fields = ['id', 'name', 'device_id', 'ip_address', 'is_online', 'created_at', 'updated_at']


class ValveControlSerializer(serializers.ModelSerializer):
    device_name = serializers.CharField(source='device.name', read_only=True)

    class Meta:
        model = ValveControl
        fields = ['id', 'device', 'device_name', 'status', 'timestamp', 'duration']


class SensorReadingSerializer(serializers.ModelSerializer):
    device_name = serializers.CharField(source='device.name', read_only=True)

    class Meta:
        model = SensorReading
        fields = ['id', 'device', 'device_name', 'flow_rate', 'timestamp']