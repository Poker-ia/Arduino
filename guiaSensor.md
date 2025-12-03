# Integración Sensor de Flujo YF-S201

## 📋 Tabla de Contenidos
1. Conexión Física
2. Código ESP32
3. Backend Django
4. Frontend React
5. Pruebas

---

## 1. 🔌 Conexión Física del Sensor YF-S201

### Pinout del Sensor YF-S201
```
Sensor YF-S201
├─ Cable ROJO → +5V
├─ Cable NEGRO → GND
└─ Cable AMARILLO → GPIO (entrada digital)
```

### Conexión en ESP32
```
YF-S201
├─ ROJO (+5V) ──────→ 5V del ESP32 (o regulador)
├─ NEGRO (GND) ─────→ GND (tierra común)
└─ AMARILLO (SIGNAL) → GPIO35 del ESP32 (entrada digital)
```

### Ubicación en el circuito de agua
```
Fuente de agua → VÁLVULA SOLENOIDE → SENSOR YF-S201 → Salida de agua
                                          ↓
                                     Mide caudal
```

**Importante:** El sensor debe estar en la tubería DESPUÉS de la válvula para medir el flujo cuando está abierta.

---

## 2. 💻 Código ESP32 Completo

```cpp
#include <WiFi.h>
#include <WebServer.h>

// ===== CONFIGURACIÓN =====
const char* ssid = "CHEYLA_2.4G";
const char* password = "123deza123";
const char* device_id = "ESP32_VALVE_001";
const char* device_name = "Válvula Principal";

// ===== PINES =====
const int VALVE_PIN = 17;      // GPIO17 - Relé
const int SENSOR_PIN = 35;     // GPIO35 - Sensor YF-S201

// ===== VARIABLES =====
WebServer server(80);
bool valve_state = false;
volatile int pulse_count = 0;
unsigned long last_time = 0;
float flow_rate = 0.0;
float total_volume = 0.0;

// ===== SENSOR INTERRUPT =====
void IRAM_ATTR sensor_interrupt() {
  pulse_count++;
}

void setup_wifi() {
  Serial.println("\n\n╔════════════════════════════════════════╗");
  Serial.println("║  ESP32 - Válvula + Sensor de Flujo    ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  Serial.print("Conectando a WiFi: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("✓ WiFi conectado!");
    Serial.print("📍 IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("✗ Error en conexión WiFi");
  }
}

void setup_gpio() {
  pinMode(VALVE_PIN, OUTPUT);
  pinMode(SENSOR_PIN, INPUT);
  
  digitalWrite(VALVE_PIN, HIGH);  // Cerrado por defecto
  
  // Configurar interrupción para el sensor
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), sensor_interrupt, RISING);
  
  Serial.println("✓ GPIO configurado:");
  Serial.println("  - VALVE_PIN (GPIO17): Relé");
  Serial.println("  - SENSOR_PIN (GPIO35): Sensor YF-S201");
}

void setup_server() {
  // Control de válvula
  server.on("/api/valve/open", HTTP_POST, handle_open_valve);
  server.on("/api/valve/close", HTTP_POST, handle_close_valve);
  
  // Estado
  server.on("/api/status", HTTP_GET, handle_status);
  server.on("/api/valve/status", HTTP_GET, handle_valve_status);
  
  // Sensor de flujo
  server.on("/api/sensor/flow", HTTP_GET, handle_sensor_flow);
  
  server.on("/", HTTP_GET, handle_root);
  server.onNotFound(handle_not_found);
  server.begin();
  
  Serial.println("✓ Servidor HTTP iniciado en puerto 80\n");
}

// ===== MANEJADORES HTTP =====

void handle_root() {
  String response = "{\"device\":\"" + String(device_name) + "\",\"status\":\"online\"}";
  server.send(200, "application/json", response);
}

void handle_open_valve() {
  Serial.println("\n📖 ABRIR VÁLVULA");
  digitalWrite(VALVE_PIN, LOW);
  valve_state = true;
  pulse_count = 0;
  total_volume = 0.0;
  
  String response = "{\"status\":\"open\",\"message\":\"Válvula abierta\"}";
  server.send(200, "application/json", response);
}

void handle_close_valve() {
  Serial.println("📕 CERRAR VÁLVULA");
  digitalWrite(VALVE_PIN, HIGH);
  valve_state = false;
  
  String response = "{\"status\":\"closed\",\"message\":\"Válvula cerrada\"}";
  server.send(200, "application/json", response);
}

void handle_status() {
  String status_str = valve_state ? "open" : "closed";
  String response = "{\"device_id\":\"" + String(device_id) + "\",\"valve_state\":\"" + status_str + "\",\"flow_rate\":" + String(flow_rate, 2) + ",\"total_volume\":" + String(total_volume, 2) + "}";
  server.send(200, "application/json", response);
}

void handle_valve_status() {
  String status_str = valve_state ? "open" : "closed";
  String response = "{\"status\":\"" + status_str + "\"}";
  server.send(200, "application/json", response);
}

void handle_sensor_flow() {
  // YF-S201: ~5.5 pulsos por mililitro
  // Cálculo: (pulsos / 5.5) = mL
  float current_volume = pulse_count / 5.5;  // en mL
  total_volume = current_volume;
  
  // Caudal en L/min: (pulsos por segundo * 60) / 5.5 / 1000
  unsigned long now = millis();
  if (now - last_time >= 1000) {
    flow_rate = (pulse_count * 60.0) / 5.5 / 1000.0;
    pulse_count = 0;
    last_time = now;
    
    Serial.print("💧 Caudal: ");
    Serial.print(flow_rate);
    Serial.print(" L/min | Total: ");
    Serial.print(total_volume);
    Serial.println(" mL");
  }
  
  String response = "{\"flow_rate\":" + String(flow_rate, 2) + ",\"total_volume\":" + String(total_volume, 2) + ",\"unit\":\"L/min\"}";
  server.send(200, "application/json", response);
}

void handle_not_found() {
  server.send(404, "application/json", "{\"error\":\"Not found\"}");
}

// ===== SETUP Y LOOP =====

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  setup_gpio();
  setup_wifi();
  setup_server();

  Serial.println("✓ Sistema listo!\n");
}

void loop() {
  server.handleClient();
  delay(10);
}
```

---

## 3. 🖥️ Backend Django

### models.py - Agregar modelo para lecturas
```python
from django.db import models

class FlowReading(models.Model):
    device = models.ForeignKey(Device, on_delete=models.CASCADE)
    flow_rate = models.FloatField(help_text="Caudal en L/min")
    total_volume = models.FloatField(help_text="Volumen total en mL")
    timestamp = models.DateTimeField(auto_now_add=True)

    class Meta:
        ordering = ['-timestamp']

    def __str__(self):
        return f"{self.device.name} - {self.flow_rate} L/min"
```

### serializers.py
```python
from rest_framework import serializers
from .models import FlowReading

class FlowReadingSerializer(serializers.ModelSerializer):
    device_name = serializers.CharField(source='device.name', read_only=True)

    class Meta:
        model = FlowReading
        fields = ['id', 'device', 'device_name', 'flow_rate', 'total_volume', 'timestamp']
```

### views.py - Agregar endpoint
```python
from rest_framework import viewsets
from .models import FlowReading
from .serializers import FlowReadingSerializer

class FlowReadingViewSet(viewsets.ModelViewSet):
    queryset = FlowReading.objects.all()
    serializer_class = FlowReadingSerializer

    def create(self, request, *args, **kwargs):
        # Guardar lectura del sensor
        device_id = request.data.get('device_id')
        flow_rate = request.data.get('flow_rate')
        total_volume = request.data.get('total_volume')
        
        device = Device.objects.get(id=device_id)
        reading = FlowReading.objects.create(
            device=device,
            flow_rate=flow_rate,
            total_volume=total_volume
        )
        serializer = self.get_serializer(reading)
        return Response(serializer.data, status=status.HTTP_201_CREATED)
```

### urls.py
```python
from rest_framework.routers import DefaultRouter
from .views import FlowReadingViewSet

router = DefaultRouter()
router.register(r'flow-readings', FlowReadingViewSet)

urlpatterns = [
    path('', include(router.urls)),
]
```

### admin.py
```python
from django.contrib import admin
from .models import FlowReading

@admin.register(FlowReading)
class FlowReadingAdmin(admin.ModelAdmin):
    list_display = ('device', 'flow_rate', 'total_volume', 'timestamp')
    list_filter = ('device', 'timestamp')
    search_fields = ('device__name',)
    readonly_fields = ('timestamp',)
```

---

## 4. ⚛️ Frontend React

### services/api.js - Agregar servicio
```javascript
export const flowAPI = {
  getLatest: (deviceId) => api.get(`/flow-readings/?device_id=${deviceId}`),
  
  getAll: () => api.get('/flow-readings/'),
  
  create: (deviceId, flowRate, totalVolume) => api.post('/flow-readings/', {
    device_id: deviceId,
    flow_rate: flowRate,
    total_volume: totalVolume
  })
};
```

### components/FlowMonitor.jsx
```jsx
import React, { useState, useEffect } from 'react';
import { flowAPI } from '../services/api';
import './FlowMonitor.css';

function FlowMonitor({ device }) {
  const [flowData, setFlowData] = useState(null);
  const [loading, setLoading] = useState(false);

  useEffect(() => {
    fetchFlowData();
    const interval = setInterval(fetchFlowData, 2000); // Actualizar cada 2 segundos
    return () => clearInterval(interval);
  }, [device]);

  const fetchFlowData = async () => {
    try {
      const response = await flowAPI.getLatest(device.id);
      if (response.data && response.data.length > 0) {
        setFlowData(response.data[0]);
      }
    } catch (err) {
      console.error('Error fetching flow data:', err);
    }
  };

  return (
    <div className="flow-monitor">
      <h3>💧 Monitor de Flujo</h3>
      
      {flowData ? (
        <div className="flow-data">
          <div className="metric">
            <label>Caudal:</label>
            <span className="value">{flowData.flow_rate?.toFixed(2) || '0.00'} L/min</span>
          </div>
          
          <div className="metric">
            <label>Volumen Total:</label>
            <span className="value">{flowData.total_volume?.toFixed(2) || '0.00'} mL</span>
          </div>
          
          <div className="metric">
            <label>Última lectura:</label>
            <span className="timestamp">
              {new Date(flowData.timestamp).toLocaleTimeString()}
            </span>
          </div>
        </div>
      ) : (
        <p>Sin datos de sensor</p>
      )}
    </div>
  );
}

export default FlowMonitor;
```

### components/FlowMonitor.css
```css
.flow-monitor {
  background: #f5f5f5;
  border-radius: 8px;
  padding: 15px;
  margin: 15px 0;
  border-left: 4px solid #667eea;
}

.flow-monitor h3 {
  color: #667eea;
  margin-bottom: 15px;
}

.flow-data {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 15px;
}

.metric {
  background: white;
  padding: 12px;
  border-radius: 6px;
  border: 1px solid #ddd;
}

.metric label {
  display: block;
  color: #666;
  font-size: 0.9em;
  margin-bottom: 5px;
}

.metric .value {
  font-size: 1.5em;
  font-weight: bold;
  color: #667eea;
}

.timestamp {
  font-size: 0.85em;
  color: #999;
}
```

---

## 5. 🧪 Pasos de Implementación

### Backend
1. Ejecuta migraciones: `python manage.py makemigrations && python manage.py migrate`
2. Regresa en admin
3. Prueba endpoint: `http://localhost:8000/api/flow-readings/`

### Frontend
1. Crea `components/FlowMonitor.jsx` con el código
2. Importa en `Dashboard.jsx`
3. Agrega `<FlowMonitor device={device} />` en el componente DeviceCard

### ESP32
1. Carga el código completo
2. Abre Serial Monitor (115200)
3. Verifica que detecte pulsos del sensor

---

## 📊 Verificación

### Serial Monitor ESP32
```
✓ Sensor YF-S201 conectado
💧 Caudal: 2.45 L/min | Total: 1250.50 mL
```

### API Response
```json
{
  "flow_rate": 2.45,
  "total_volume": 1250.50,
  "unit": "L/min"
}
```

---

## ⚡ Calibración del Sensor

El YF-S201 tiene un factor de conversión de **5.5 pulsos por mililitro**.

Si necesitas ajustar:
```cpp
// Cambiar este valor si los datos no son precisos
float SENSOR_FACTOR = 5.5;  // pulsos/mL
```

Pueba: Deja fluir 1 litro exacto y cuenta los pulsos, luego: `SENSOR_FACTOR = pulsos / 1000`

---

## 🔌 Troubleshooting

| Problema | Solución |
|----------|----------|
| No hay lecturas | Verifica GPIO35, sensor conectado a +5V |
| Lecturas erráticas | Revisa conexión sensor, cables cortos |
| Flow rate muy alto/bajo | Calibra el SENSOR_FACTOR |
| ESP32 se reinicia | Bajo voltaje, verifica alimentación |