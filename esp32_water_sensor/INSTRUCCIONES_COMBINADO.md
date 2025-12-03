# Código Combinado: Válvula + Sensor de Flujo

## 📋 Cambios Realizados

He combinado tu código funcional de la válvula con el sensor de flujo de agua YF-S201.

### ✨ Nuevas Funcionalidades

1. **Medición de caudal** en tiempo real (L/min)
2. **Cálculo de consumo total** acumulado (litros)
3. **Envío automático** de datos al backend cada 5 segundos
4. **Detección de pulsos** mediante interrupciones

### 🔧 Configuración Necesaria

Antes de cargar el código, **DEBES CAMBIAR** esta línea con la IP de tu PC:

```cpp
const char* backendURL = "http://192.168.1.100:8000/api/sensor-readings/";
```

**Para obtener la IP de tu PC:**

**Windows:**
```bash
ipconfig
```
Busca "Dirección IPv4" en la sección de tu adaptador WiFi.

**Ejemplo:** Si tu IP es `192.168.1.105`, la línea debe quedar:
```cpp
const char* backendURL = "http://192.168.1.105:8000/api/sensor-readings/";
```

---

## 🔌 Conexiones del Hardware

### Sensor YF-S201
| Cable Sensor | Conexión ESP32 |
|--------------|----------------|
| 🔴 ROJO      | VIN (5V)       |
| ⚫ NEGRO     | GND            |
| 🟡 AMARILLO  | GPIO2          |

### Válvula (ya conectada)
| Componente | Pin |
|------------|-----|
| Relé       | GPIO17 |

---

## 📚 Librería Adicional Requerida

Necesitas instalar **ArduinoJson** para enviar datos al backend:

1. Abrir Arduino IDE
2. Ir a **Herramientas** → **Administrar Librerías**
3. Buscar: `ArduinoJson`
4. Instalar versión **6.x** (by Benoit Blanchon)

---

## 🚀 Cómo Usar

### 1. Cargar el Código

1. Abrir `esp32_valve_sensor_combined.ino` en Arduino IDE
2. **Cambiar la IP del backend** (línea 14)
3. Verificar que WiFi SSID y password sean correctos
4. Cargar al ESP32

### 2. Monitor Serial

Abre el Monitor Serial (115200 baud) y verás:

```
╔════════════════════════════════════════╗
║  ESP32 - Control de Válvula + Sensor   ║
╚════════════════════════════════════════╝

Conectando a WiFi: OPPO
✓ WiFi conectado!
📍 IP: 192.168.1.150
Signal: -45
✓ GPIO configurado:
  - VALVE_PIN (GPIO17): HIGH - CERRADO (por defecto)
  - SENSOR_PIN (GPIO2): Configurado con interrupción
✓ Servidor HTTP iniciado en puerto 80

✓ Sistema listo! Esperando comandos...
💧 Sensor de flujo activo - esperando flujo de agua...
```

### 3. Cuando Hay Flujo de Agua

```
─────────────────────────────
💧 Caudal: 5.23 L/min
📊 Consumo Total: 123.45 L
📈 Pulsos: 39
💧 Datos enviados al backend (HTTP 201)
   ✓ Lectura registrada exitosamente
```

---

## 🧪 Pruebas

### Probar Válvula (ya funciona)
Desde el frontend o con curl:
```bash
curl -X POST http://IP_DEL_ESP32/api/valve/open
curl -X POST http://IP_DEL_ESP32/api/valve/close
```

### Probar Sensor
1. Abrir la válvula desde el frontend
2. Dejar fluir agua por el sensor
3. Ver en Monitor Serial los datos de caudal
4. Verificar en el frontend que aparecen los datos

### Verificar Envío al Backend
En el Monitor Serial deberías ver cada 5 segundos:
```
💧 Datos enviados al backend (HTTP 201)
   ✓ Lectura registrada exitosamente
```

Si ves error, verifica:
- ✅ Backend corriendo en `http://localhost:8000`
- ✅ IP correcta en el código
- ✅ Dispositivo registrado en Django Admin con `device_id = "ESP32_VALVE_001"`

---

## 📊 Datos Enviados al Backend

Cada 5 segundos el ESP32 envía:

```json
{
  "device_id": "ESP32_VALVE_001",
  "flow_rate": 5.23,
  "total_volume": 123.45
}
```

---

## ⚙️ Calibración del Sensor

Si las mediciones no son precisas:

1. Medir un volumen conocido (ej: 10 litros)
2. Dejar fluir y anotar el consumo total mostrado
3. Calcular: `factor_nuevo = (volumen_real / volumen_medido) * 7.5`
4. Actualizar en el código (línea 28):

```cpp
const float calibrationFactor = 7.5;  // Cambiar por el nuevo factor
```

---

## 🐛 Solución de Problemas

### No se envían datos al backend
**Error:** `✗ Error al enviar datos: connection refused`

**Soluciones:**
1. Verificar que el backend esté corriendo
2. Verificar la IP en `backendURL`
3. Asegurar que ESP32 y PC estén en la misma red WiFi
4. Desactivar firewall temporalmente

### No detecta flujo de agua
**Síntomas:** Caudal siempre en 0.00 L/min

**Soluciones:**
1. Verificar conexión del cable amarillo a GPIO2
2. Verificar que el sensor esté correctamente instalado
3. Asegurar flujo mínimo de ~1 L/min (el sensor no detecta flujos muy bajos)
4. Revisar que el sensor esté alimentado (cable rojo a VIN)

### Error al compilar
**Error:** `ArduinoJson.h: No such file or directory`

**Solución:** Instalar librería ArduinoJson v6.x desde el Administrador de Librerías

---

## 📝 Diferencias con tu Código Original

| Característica | Código Original | Código Nuevo |
|----------------|-----------------|--------------|
| Control de válvula | ✅ | ✅ |
| Sensor de flujo | ❌ | ✅ |
| Envío a backend | ❌ | ✅ |
| Medición de consumo | ❌ | ✅ |
| Librería ArduinoJson | ❌ | ✅ (requerida) |

---

## ✅ Checklist de Instalación

- [ ] Instalar librería ArduinoJson v6.x
- [ ] Cambiar IP del backend en línea 14
- [ ] Verificar WiFi SSID y password
- [ ] Conectar cable amarillo del sensor a GPIO2
- [ ] Conectar cable rojo a VIN (5V)
- [ ] Conectar cable negro a GND
- [ ] Cargar código al ESP32
- [ ] Abrir Monitor Serial (115200 baud)
- [ ] Verificar conexión WiFi exitosa
- [ ] Registrar dispositivo en Django Admin
- [ ] Probar flujo de agua
- [ ] Verificar datos en el frontend

---

¡Listo! Ahora tu ESP32 controla la válvula Y mide el consumo de agua simultáneamente. 🎉
