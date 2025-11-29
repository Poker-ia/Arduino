from django.db import models

class Device(models.Model):
    """Modelo para dispositivos ESP32"""
    name = models.CharField(max_length=100)
    device_id = models.CharField(max_length=100, unique=True)
    ip_address = models.GenericIPAddressField()
    is_online = models.BooleanField(default=False)
    created_at = models.DateTimeField(auto_now_add=True)
    updated_at = models.DateTimeField(auto_now=True)

    def __str__(self):
        return self.name


class ValveControl(models.Model):
    """Modelo para registrar control de válvulas"""
    STATUS_CHOICES = [
        ('open', 'Abierta'),
        ('closed', 'Cerrada'),
    ]

    device = models.ForeignKey(Device, on_delete=models.CASCADE)
    status = models.CharField(max_length=10, choices=STATUS_CHOICES)
    timestamp = models.DateTimeField(auto_now_add=True)
    duration = models.IntegerField(null=True, blank=True, help_text="Duración en segundos")

    class Meta:
        ordering = ['-timestamp']

    def __str__(self):
        return f"{self.device.name} - {self.status}"


class SensorReading(models.Model):
    """Modelo para lecturas del sensor YF-S201"""
    device = models.ForeignKey(Device, on_delete=models.CASCADE)
    flow_rate = models.FloatField(help_text="Caudal en L/min")
    timestamp = models.DateTimeField(auto_now_add=True)

    class Meta:
        ordering = ['-timestamp']

    def __str__(self):
        return f"{self.device.name} - {self.flow_rate} L/min"