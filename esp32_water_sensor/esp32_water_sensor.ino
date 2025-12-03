#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WebServer.h>

// ========== CONFIGURACIÓN WiFi ==========
const char* ssid = "TU_RED_WIFI";           // Cambiar por tu red WiFi
const char* password = "TU_PASSWORD_WIFI";   // Cambiar por tu contraseña

// ========== CONFIGURACIÓN Backend ==========
const char* backendURL = "http://192.168.1.100:8000/api/sensor-readings/";  // Cambiar IP del backend
const char* deviceID = "ESP32_001";  // ID único del dispositivo

// ========== PINES ==========
const int SENSOR_PIN = 2;      // Pin del sensor YF-S201 (GPIO2)
const int VALVE_PIN = 4;       // Pin del relé para válvula (GPIO4)

// ========== VARIABLES DEL SENSOR ==========
volatile int pulseCount = 0;   // Contador de pulsos (volátil para interrupciones)
float flowRate = 0.0;          // Caudal en L/min
float totalVolume = 0.0;       // Volumen total acumulado en litros
unsigned long oldTime = 0;     // Tiempo anterior para cálculos

// Calibración del sensor YF-S201
// Típicamente: 7.5 pulsos/segundo = 1 L/min
// Ajustar según calibración real
const float calibrationFactor = 7.5;

// ========== SERVIDOR WEB ==========
WebServer server(80);

// ========== FUNCIÓN DE INTERRUPCIÓN ==========
void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=== ESP32 Water Sensor System ===");
  
  // Configurar pines
  pinMode(SENSOR_PIN, INPUT_PULLUP);
  pinMode(VALVE_PIN, OUTPUT);
  digitalWrite(VALVE_PIN, LOW);  // Válvula cerrada por defecto
  
  // Configurar interrupción para el sensor
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), pulseCounter, FALLING);
  
  // Conectar a WiFi
  Serial.print("Conectando a WiFi");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\n✓ WiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  
  // Configurar rutas del servidor web
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/valve/open", HTTP_POST, handleOpenValve);
  server.on("/api/valve/close", HTTP_POST, handleCloseValve);
  
  server.begin();
  Serial.println("✓ Servidor HTTP iniciado");
  
  oldTime = millis();
}

// ========== LOOP PRINCIPAL ==========
void loop() {
  server.handleClient();  // Manejar peticiones HTTP
  
  // Calcular caudal cada 1 segundo
  if ((millis() - oldTime) > 1000) {
    // Deshabilitar interrupciones temporalmente
    detachInterrupt(digitalPinToInterrupt(SENSOR_PIN));
    
    // Calcular caudal (L/min)
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    
    // Calcular volumen acumulado
    // Volumen = caudal (L/min) * tiempo (min)
    float timeElapsed = (millis() - oldTime) / 60000.0;  // Convertir a minutos
    totalVolume += flowRate * timeElapsed;
    
    oldTime = millis();
    pulseCount = 0;
    
    // Reactivar interrupciones
    attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), pulseCounter, FALLING);
    
    // Mostrar en monitor serial
    Serial.println("─────────────────────────────");
    Serial.print("Caudal: ");
    Serial.print(flowRate, 2);
    Serial.println(" L/min");
    Serial.print("Consumo Total: ");
    Serial.print(totalVolume, 2);
    Serial.println(" L");
    
    // Enviar datos al backend cada 5 segundos
    static unsigned long lastSend = 0;
    if (millis() - lastSend > 5000) {
      sendDataToBackend();
      lastSend = millis();
    }
  }
}

// ========== ENVIAR DATOS AL BACKEND ==========
void sendDataToBackend() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(backendURL);
    http.addHeader("Content-Type", "application/json");
    
    // Crear JSON
    StaticJsonDocument<200> doc;
    doc["device_id"] = deviceID;
    doc["flow_rate"] = flowRate;
    doc["total_volume"] = totalVolume;
    
    String jsonData;
    serializeJson(doc, jsonData);
    
    // Enviar POST
    int httpCode = http.POST(jsonData);
    
    if (httpCode > 0) {
      Serial.print("✓ Datos enviados al backend (HTTP ");
      Serial.print(httpCode);
      Serial.println(")");
      
      if (httpCode == 201) {
        Serial.println("  Lectura registrada exitosamente");
      }
    } else {
      Serial.print("✗ Error al enviar datos: ");
      Serial.println(http.errorToString(httpCode));
    }
    
    http.end();
  } else {
    Serial.println("✗ WiFi desconectado");
  }
}

// ========== ENDPOINT: /api/status ==========
void handleStatus() {
  StaticJsonDocument<300> doc;
  doc["device_id"] = deviceID;
  doc["flow_rate"] = flowRate;
  doc["total_volume"] = totalVolume;
  doc["valve_status"] = digitalRead(VALVE_PIN) ? "open" : "closed";
  doc["wifi_rssi"] = WiFi.RSSI();
  doc["uptime"] = millis() / 1000;
  
  String response;
  serializeJson(doc, response);
  
  server.send(200, "application/json", response);
  Serial.println("→ Status solicitado");
}

// ========== ENDPOINT: /api/valve/open ==========
void handleOpenValve() {
  digitalWrite(VALVE_PIN, HIGH);
  Serial.println("→ Válvula ABIERTA");
  
  StaticJsonDocument<100> doc;
  doc["status"] = "open";
  doc["message"] = "Válvula abierta";
  
  String response;
  serializeJson(doc, response);
  
  server.send(200, "application/json", response);
}

// ========== ENDPOINT: /api/valve/close ==========
void handleCloseValve() {
  digitalWrite(VALVE_PIN, LOW);
  Serial.println("→ Válvula CERRADA");
  
  StaticJsonDocument<100> doc;
  doc["status"] = "closed";
  doc["message"] = "Válvula cerrada";
  
  String response;
  serializeJson(doc, response);
  
  server.send(200, "application/json", response);
}
