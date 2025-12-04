from django.core.management.base import BaseCommand
from api.models import Device, SensorReading


class Command(BaseCommand):
    help = 'Verifica y crea el dispositivo ESP32 para recibir datos del sensor'

    def handle(self, *args, **options):
        self.stdout.write("\n" + "="*60)
        self.stdout.write("  VERIFICACIÓN DE DISPOSITIVO Y SENSOR")
        self.stdout.write("="*60 + "\n")

        # Configuración del dispositivo
        DEVICE_ID = "ESP32_VALVE_001"
        DEVICE_NAME = "Válvula Principal"
        DEVICE_IP = "10.60.136.30"  # IP del ESP32

        # 1. Verificar/crear dispositivo
        self.stdout.write("1️⃣  Verificando dispositivo...")
        device, created = Device.objects.get_or_create(
            device_id=DEVICE_ID,
            defaults={
                'name': DEVICE_NAME,
                'ip_address': DEVICE_IP,
                'is_online': True
            }
        )

        if created:
            self.stdout.write(self.style.SUCCESS(f"   ✅ Dispositivo CREADO: {device.name}"))
        else:
            self.stdout.write(self.style.WARNING(f"   ℹ️  Dispositivo ya existe: {device.name}"))
            # Actualizar IP y estado online
            device.ip_address = DEVICE_IP
            device.is_online = True
            device.save()
            self.stdout.write(self.style.SUCCESS(f"   ✅ IP actualizada a: {DEVICE_IP}"))

        self.stdout.write(f"   📍 ID en BD: {device.id}")
        self.stdout.write(f"   🆔 device_id: {device.device_id}")
        self.stdout.write(f"   🌐 IP: {device.ip_address}")
        self.stdout.write(f"   📶 Online: {'✅ Sí' if device.is_online else '❌ No'}")

        # 2. Verificar tabla SensorReading
        self.stdout.write("\n2️⃣  Verificando tabla SensorReading...")
        total_readings = SensorReading.objects.count()
        device_readings = SensorReading.objects.filter(device=device).count()

        self.stdout.write(f"   📊 Total de lecturas en BD: {total_readings}")
        self.stdout.write(f"   📊 Lecturas de este dispositivo: {device_readings}")

        # 3. Mostrar últimas 5 lecturas
        if device_readings > 0:
            self.stdout.write("\n3️⃣  Últimas 5 lecturas:")
            latest_readings = SensorReading.objects.filter(device=device)[:5]
            for reading in latest_readings:
                self.stdout.write(
                    f"   • {reading.timestamp.strftime('%Y-%m-%d %H:%M:%S')} - "
                    f"Caudal: {reading.flow_rate:.2f} L/min - "
                    f"Total: {reading.total_volume:.2f} L"
                )
        else:
            self.stdout.write("\n3️⃣  No hay lecturas aún")
            self.stdout.write(self.style.WARNING("   ⏳ Esperando datos del ESP32..."))

        # 4. Resumen
        self.stdout.write("\n" + "="*60)
        self.stdout.write("  RESUMEN")
        self.stdout.write("="*60)
        self.stdout.write(self.style.SUCCESS(f"✅ Dispositivo registrado: {device.device_id}"))
        self.stdout.write(self.style.SUCCESS(f"✅ Endpoint disponible: /api/sensor-readings/"))
        self.stdout.write(self.style.SUCCESS(f"✅ El ESP32 puede enviar datos ahora"))
        self.stdout.write("\n💡 Reinicia el ESP32 para que comience a enviar datos")
        self.stdout.write("   El ESP32 enviará a: http://10.60.136.30:8000/api/sensor-readings/")
        self.stdout.write("="*60 + "\n")
