#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ===== CONFIGURACIÓN WiFi =====
const char* ssid = "OPPO";
const char* password = "jairo233";
const char* device_id = "ESP32_VALVE_001";
const char* device_name = "Válvula Principal";

// ===== CONFIGURACIÓN Backend =====
const char* backendURL = "http://10.60.136.30:8000/api/sensor-readings/";  // ✅ IP de tu PC

// ===== PINES =====
const int VALVE_PIN = 17;      // GPIO17 para controlar válvula
const int SENSOR_PIN = 2;      // GPIO2 para sensor YF-S201

// ===== VARIABLES VÁLVULA =====
WebServer server(80);
bool valve_state = false;

// ===== VARIABLES SENSOR =====
volatile int pulseCount = 0;   // Contador de pulsos (volátil para interrupciones)
float flowRate = 0.0;          // Caudal en L/min
float totalVolume = 0.0;       // Volumen total acumulado en litros
unsigned long oldTime = 0;     // Tiempo anterior para cálculos

// Calibración del sensor YF-S201
// Típicamente: 7.5 pulsos/segundo = 1 L/min
const float calibrationFactor = 7.5;

// ===== FUNCIÓN DE INTERRUPCIÓN DEL SENSOR =====
void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

// ===== ENVIAR DATOS AL BACKEND =====
void sendDataToBackend() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(backendURL);
    http.addHeader("Content-Type", "application/json");
    
    // Crear JSON
    StaticJsonDocument<200> doc;
    doc["device_id"] = device_id;
    doc["flow_rate"] = flowRate;
    doc["total_volume"] = totalVolume;
    
    String jsonData;
    serializeJson(doc, jsonData);
    
    // Enviar POST
    int httpCode = http.POST(jsonData);
    
    if (httpCode > 0) {
      Serial.print("💧 Datos enviados al backend (HTTP ");
      Serial.print(httpCode);
      Serial.println(")");
      
      if (httpCode == 201) {
        Serial.println("   ✓ Lectura registrada exitosamente");
      }
    } else {
      Serial.print("   ✗ Error al enviar datos: ");
      Serial.println(http.errorToString(httpCode));
    }
    
    http.end();
  } else {
    Serial.println("✗ WiFi desconectado - no se pueden enviar datos");
  }
}

void setup_wifi() {
  Serial.println("\n\n╔════════════════════════════════════════╗");
  Serial.println("║  ESP32 - Control de Válvula + Sensor   ║");
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
    Serial.print("Signal: ");
    Serial.println(WiFi.RSSI());
  } else {
    Serial.println("✗ Error en conexión WiFi");
  }
}

void setup_gpio() {
  // Configurar válvula
  pinMode(VALVE_PIN, OUTPUT);
  digitalWrite(VALVE_PIN, HIGH);  // Cerrado por defecto (HIGH = CERRADO)
  
  // Configurar sensor
  pinMode(SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), pulseCounter, FALLING);
  
  Serial.println("✓ GPIO configurado:");
  Serial.print("  - VALVE_PIN (GPIO17): ");
  Serial.println(digitalRead(VALVE_PIN) == HIGH ? "HIGH - CERRADO (por defecto)" : "LOW - ABIERTO");
  Serial.println("  - SENSOR_PIN (GPIO2): Configurado con interrupción");
}

void setup_server() {
  server.on("/api/valve/open", HTTP_POST, handle_open_valve);
  server.on("/api/valve/close", HTTP_POST, handle_close_valve);
  server.on("/api/status", HTTP_GET, handle_status);
  server.on("/api/valve/status", HTTP_GET, handle_valve_status);
  server.on("/", HTTP_GET, handle_root);
  server.onNotFound(handle_not_found);
  server.begin();
  
  Serial.println("✓ Servidor HTTP iniciado en puerto 80");
  Serial.println("\n═══════════════════════════════════════\n");
}

void handle_root() {
  String response = "{\"device\":\"" + String(device_name) + "\",\"id\":\"" + String(device_id) + "\",\"status\":\"online\"}";
  server.send(200, "application/json", response);
  Serial.println("📡 GET / - Dispositivo en línea");
}

void handle_open_valve() {
  Serial.println("\n┌─────────────────────────────────┐");
  Serial.println("│ 📖 PETICIÓN: ABRIR VÁLVULA      │");
  Serial.println("└─────────────────────────────────┘");
  
  Serial.print("  Estado ANTES: ");
  Serial.println(digitalRead(VALVE_PIN) == HIGH ? "HIGH - CON CORRIENTE" : "LOW - SIN CORRIENTE");
  
  digitalWrite(VALVE_PIN, LOW);  // LOW para ABRIR
  valve_state = true;
  
  delay(100);
  
  Serial.print("  Estado DESPUÉS: ");
  int state = digitalRead(VALVE_PIN);
  Serial.println(state == HIGH ? "HIGH - CON CORRIENTE ✅" : "LOW - SIN CORRIENTE ❌");
  Serial.println("  LED verde relé: DEBE estar PRENDIDO\n");
  
  String response = "{\"status\":\"open\",\"message\":\"Válvula abierta\"}";
  server.send(200, "application/json", response);
  Serial.println("✓ Respuesta enviada a Django\n");
}

void handle_close_valve() {
  Serial.println("\n┌─────────────────────────────────┐");
  Serial.println("│ 📕 PETICIÓN: CERRAR VÁLVULA     │");
  Serial.println("└─────────────────────────────────┘");
  
  Serial.print("  Estado ANTES: ");
  Serial.println(digitalRead(VALVE_PIN) == HIGH ? "HIGH - CON CORRIENTE" : "LOW - SIN CORRIENTE");
  
  digitalWrite(VALVE_PIN, HIGH);  // HIGH para CERRAR
  valve_state = false;
  
  delay(100);
  
  Serial.print("  Estado DESPUÉS: ");
  int state = digitalRead(VALVE_PIN);
  Serial.println(state == LOW ? "LOW - SIN CORRIENTE ✅" : "HIGH - CON CORRIENTE ❌");
  Serial.println("  LED verde relé: DEBE estar APAGADO\n");
  
  String response = "{\"status\":\"closed\",\"message\":\"Válvula cerrada\"}";
  server.send(200, "application/json", response);
  Serial.println("✓ Respuesta enviada a Django\n");
}

void handle_status() {
  String status_str = valve_state ? "open" : "closed";
  int gpio_state = digitalRead(VALVE_PIN);
  
  Serial.println("📡 GET /api/status consultado");
  Serial.print("   Estado válvula: ");
  Serial.print(status_str);
  Serial.print(" - GPIO17: ");
  Serial.println(gpio_state == HIGH ? "HIGH" : "LOW");
  
  String response = "{\"device_id\":\"" + String(device_id) + 
                    "\",\"device_name\":\"" + String(device_name) + 
                    "\",\"valve_state\":\"" + status_str + 
                    "\",\"gpio_state\":" + String(gpio_state) + 
                    ",\"flow_rate\":" + String(flowRate, 2) + 
                    ",\"total_volume\":" + String(totalVolume, 2) + 
                    ",\"wifi_signal\":" + String(WiFi.RSSI()) + "}";
  server.send(200, "application/json", response);
}

void handle_valve_status() {
  String status_str = valve_state ? "open" : "closed";
  int gpio_state = digitalRead(VALVE_PIN);
  
  String response = "{\"status\":\"" + status_str + "\",\"gpio_state\":" + String(gpio_state) + "}";
  server.send(200, "application/json", response);
}

void handle_not_found() {
  server.send(404, "application/json", "{\"error\":\"Endpoint no encontrado\"}");
  Serial.print("⚠️  404 - Endpoint no encontrado: ");
  Serial.println(server.uri());
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  setup_gpio();
  setup_wifi();
  setup_server();

  oldTime = millis();
  
  Serial.println("✓ Sistema listo! Esperando comandos...\n");
  Serial.println("💧 Sensor de flujo activo - esperando flujo de agua...\n");
}

void loop() {
  server.handleClient();
  
  // ===== CALCULAR CAUDAL CADA 1 SEGUNDO =====
  if ((millis() - oldTime) > 1000) {
    // Deshabilitar interrupciones temporalmente
    detachInterrupt(digitalPinToInterrupt(SENSOR_PIN));
    
    // Calcular caudal (L/min)
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    
    // Calcular volumen acumulado
    float timeElapsed = (millis() - oldTime) / 60000.0;  // Convertir a minutos
    totalVolume += flowRate * timeElapsed;
    
    oldTime = millis();
    
    // Mostrar en monitor serial solo si hay flujo
    if (flowRate > 0.1 || pulseCount > 0) {
      Serial.println("─────────────────────────────");
      Serial.print("💧 Caudal: ");
      Serial.print(flowRate, 2);
      Serial.println(" L/min");
      Serial.print("📊 Consumo Total: ");
      Serial.print(totalVolume, 2);
      Serial.println(" L");
      Serial.print("📈 Pulsos: ");
      Serial.println(pulseCount);
    }
    
    pulseCount = 0;
    
    // Reactivar interrupciones
    attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), pulseCounter, FALLING);
    
    // ===== ENVIAR DATOS AL BACKEND CADA 5 SEGUNDOS =====
    static unsigned long lastSend = 0;
    if (millis() - lastSend > 5000) {
      sendDataToBackend();
      lastSend = millis();
    }
  }
  
  delay(10);
}
