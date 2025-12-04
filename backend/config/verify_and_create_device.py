"""
Script para verificar y crear el dispositivo ESP32 en la base de datos
Ejecutar con: python manage.py shell < verify_and_create_device.py
"""

from api.models import Device, SensorReading
from django.utils import timezone

print("\n" + "="*60)
print("  VERIFICACIÓN DE DISPOSITIVO Y SENSOR")
print("="*60 + "\n")

# Configuración del dispositivo
DEVICE_ID = "ESP32_VALVE_001"
DEVICE_NAME = "Válvula Principal"
DEVICE_IP = "10.60.136.30"  # Cambiar a la IP real del ESP32

# 1. Verificar si el dispositivo existe
print("1️⃣  Verificando dispositivo...")
device, created = Device.objects.get_or_create(
    device_id=DEVICE_ID,
    defaults={
        'name': DEVICE_NAME,
        'ip_address': DEVICE_IP,
        'is_online': True
    }
)

if created:
    print(f"   ✅ Dispositivo CREADO: {device.name}")
else:
    print(f"   ℹ️  Dispositivo ya existe: {device.name}")

print(f"   📍 ID: {device.id}")
print(f"   🆔 device_id: {device.device_id}")
print(f"   🌐 IP: {device.ip_address}")
print(f"   📶 Online: {'✅ Sí' if device.is_online else '❌ No'}")

# 2. Verificar tabla SensorReading
print("\n2️⃣  Verificando tabla SensorReading...")
total_readings = SensorReading.objects.count()
device_readings = SensorReading.objects.filter(device=device).count()

print(f"   📊 Total de lecturas en BD: {total_readings}")
print(f"   📊 Lecturas de este dispositivo: {device_readings}")

# 3. Mostrar últimas 5 lecturas
if device_readings > 0:
    print("\n3️⃣  Últimas 5 lecturas:")
    latest_readings = SensorReading.objects.filter(device=device)[:5]
    for reading in latest_readings:
        print(f"   • {reading.timestamp.strftime('%Y-%m-%d %H:%M:%S')} - "
              f"Caudal: {reading.flow_rate:.2f} L/min - "
              f"Total: {reading.total_volume:.2f} L")
else:
    print("\n3️⃣  No hay lecturas aún")
    print("   ⏳ Esperando datos del ESP32...")

# 4. Crear lectura de prueba
print("\n4️⃣  ¿Crear lectura de prueba? (para verificar que funciona)")
print("   Ejecuta esto manualmente si quieres probar:")
print(f"   SensorReading.objects.create(device_id={device.id}, flow_rate=2.5, total_volume=10.0)")

print("\n" + "="*60)
print("  RESUMEN")
print("="*60)
print(f"✅ Dispositivo registrado: {device.device_id}")
print(f"✅ Endpoint disponible: /api/sensor-readings/")
print(f"✅ El ESP32 puede enviar datos ahora")
print("\n💡 Reinicia el ESP32 para que comience a enviar datos")
print("="*60 + "\n")
