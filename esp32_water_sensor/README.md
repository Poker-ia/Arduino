# Código ESP32 - Sistema de Control de Agua

## 📋 Descripción

Este código controla un sensor de flujo de agua YF-S201 y una válvula solenoide mediante un ESP32, enviando datos automáticamente a un backend Django.

## 🔧 Hardware Requerido

- **ESP32** (cualquier modelo)
- **Sensor de flujo YF-S201**
- **Válvula solenoide** (12V o 24V según modelo)
- **Módulo relé** (para controlar la válvula)
- **Fuente de alimentación** (para válvula y ESP32)

## 📌 Conexiones

### Sensor YF-S201
- **Cable ROJO**: +5V (VIN del ESP32)
- **Cable NEGRO**: GND
- **Cable AMARILLO**: GPIO2 (pin de señal)

### Módulo Relé
- **VCC**: 3.3V o 5V del ESP32
- **GND**: GND
- **IN**: GPIO4
- **COM**: Positivo de fuente de válvula
- **NO**: Positivo de válvula solenoide

### Válvula Solenoide
- **Positivo**: Conectar a NO del relé
- **Negativo**: GND de fuente externa

## ⚙️ Configuración

Antes de cargar el código, modifica estas líneas:

```cpp
const char* ssid = "TU_RED_WIFI";           // Tu red WiFi
const char* password = "TU_PASSWORD_WIFI";   // Tu contraseña WiFi
const char* backendURL = "http://192.168.1.100:8000/api/sensor-readings/";  // IP de tu backend
const char* deviceID = "ESP32_001";  // ID único del dispositivo
```

## 📚 Librerías Necesarias

Instalar desde el Administrador de Librerías de Arduino IDE:

1. **ArduinoJson** (by Benoit Blanchon) - v6.x
2. Las demás librerías vienen incluidas con el soporte de ESP32

## 🚀 Instalación

1. Instalar soporte para ESP32 en Arduino IDE:
   - Archivo → Preferencias
   - URLs Adicionales: `https://dl.espressif.com/dl/package_esp32_index.json`
   - Herramientas → Placa → Gestor de Tarjetas → Buscar "ESP32" e instalar

2. Seleccionar placa:
   - Herramientas → Placa → ESP32 Dev Module

3. Configurar parámetros:
   - Upload Speed: 115200
   - Flash Frequency: 80MHz
   - Flash Mode: QIO
   - Flash Size: 4MB

4. Conectar ESP32 via USB y seleccionar puerto COM

5. Cargar el código

## 📊 Funcionamiento

### Medición de Caudal
- El sensor YF-S201 genera pulsos proporcionales al flujo
- Calibración típica: **7.5 pulsos/segundo = 1 L/min**
- El código calcula:
  - **Caudal actual** (L/min)
  - **Volumen total acumulado** (litros)

### Envío de Datos
- Cada **5 segundos** envía datos al backend Django
- Formato JSON:
```json
{
  "device_id": "ESP32_001",
  "flow_rate": 5.2,
  "total_volume": 123.5
}
```

### Servidor HTTP Local
El ESP32 expone estos endpoints:

- **GET** `/api/status` - Estado del dispositivo
- **POST** `/api/valve/open` - Abrir válvula
- **POST** `/api/valve/close` - Cerrar válvula

## 🔍 Monitor Serial

Abre el monitor serial (115200 baud) para ver:

```
=== ESP32 Water Sensor System ===
Conectando a WiFi....
✓ WiFi conectado!
IP: 192.168.1.150
✓ Servidor HTTP iniciado
─────────────────────────────
Caudal: 5.23 L/min
Consumo Total: 123.45 L
✓ Datos enviados al backend (HTTP 201)
  Lectura registrada exitosamente
```

## 🎯 Calibración del Sensor

El factor de calibración puede variar. Para calibrar:

1. Medir un volumen conocido (ej: 10 litros)
2. Dejar fluir el agua y contar pulsos totales
3. Calcular: `factor = pulsos_totales / volumen_litros`
4. Actualizar en el código:

```cpp
const float calibrationFactor = 7.5;  // Ajustar según tu calibración
```

## ⚠️ Notas Importantes

- **NO** conectar la válvula directamente al ESP32, usar siempre un relé
- La válvula requiere fuente externa (12V o 24V según modelo)
- Verificar que el sensor esté correctamente instalado en la tubería
- El flujo mínimo detectable del YF-S201 es ~1 L/min

## 🐛 Solución de Problemas

### No se conecta a WiFi
- Verificar SSID y contraseña
- Asegurar que la red sea 2.4GHz (ESP32 no soporta 5GHz)

### No envía datos al backend
- Verificar IP del backend
- Asegurar que el backend esté corriendo
- Revisar firewall

### Lecturas incorrectas
- Verificar conexiones del sensor
- Calibrar el factor de calibración
- Asegurar flujo mínimo de 1 L/min
