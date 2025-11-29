from django.contrib import admin
from .models import Device, ValveControl, SensorReading

# ✅ Registrar Device (Dispositivos ESP32)
@admin.register(Device)
class DeviceAdmin(admin.ModelAdmin):
    list_display = ('name', 'device_id', 'ip_address', 'is_online', 'created_at')
    list_filter = ('is_online', 'created_at')
    search_fields = ('name', 'device_id', 'ip_address')
    readonly_fields = ('created_at', 'updated_at')
    
    fieldsets = (
        ('Información del Dispositivo', {
            'fields': ('name', 'device_id', 'ip_address')
        }),
        ('Estado', {
            'fields': ('is_online',)
        }),
        ('Timestamps', {
            'fields': ('created_at', 'updated_at'),
            'classes': ('collapse',)
        }),
    )


# ✅ Registrar ValveControl (Historial de válvula)
@admin.register(ValveControl)
class ValveControlAdmin(admin.ModelAdmin):
    list_display = ('device', 'status', 'timestamp', 'duration')
    list_filter = ('status', 'timestamp', 'device')
    search_fields = ('device__name', 'device__device_id')
    readonly_fields = ('timestamp',)
    
    fieldsets = (
        ('Información', {
            'fields': ('device', 'status', 'timestamp')
        }),
        ('Detalles', {
            'fields': ('duration',),
            'classes': ('collapse',)
        }),
    )


# ✅ Registrar SensorReading (Lecturas del sensor)
@admin.register(SensorReading)
class SensorReadingAdmin(admin.ModelAdmin):
    list_display = ('device', 'flow_rate', 'timestamp')
    list_filter = ('device', 'timestamp')
    search_fields = ('device__name', 'device__device_id')
    readonly_fields = ('timestamp',)
    
    fieldsets = (
        ('Lectura', {
            'fields': ('device', 'flow_rate', 'timestamp')
        }),
    )