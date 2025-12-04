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
const char* backendURL = "http://10.60.136.11:8000/api/sensor-readings/";  // ✅ IP CORRECTA de tu PC

// ===== PINES =====
const int VALVE_PIN = 17;      // GPIO17 para controlar válvula
const int SENSOR_PIN = 35;     // GPIO35 para sensor YF-S201 (INPUT ONLY)

// ===== VARIABLES VÁLVULA =====
WebServer server(80);
bool valve_state = false;

// ===== VARIABLES SENSOR =====
volatile int pulseCount = 0;   // Contador de pulsos (volátil para interrupciones)
float flowRate = 0.0;          // Caudal en L/min
float totalVolume = 0.0;       // Volumen total acumulado en litros
unsigned long oldTime = 0;     // Tiempo anterior para cálculos

// Calibración del sensor YF-S201
const float calibrationFactor = 7.5;

// ===== DEBUG =====
bool debugMode = true;  // Activar/desactivar mensajes de debug

// ===== FUNCIÓN DE INTERRUPCIÓN DEL SENSOR =====
void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

// ===== ENVIAR DATOS AL BACKEND =====
void sendDataToBackend() {
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║     INTENTANDO ENVIAR DATOS            ║");
  Serial.println("╚════════════════════════════════════════╝");
  
  // Verificar WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ WiFi desconectado - no se pueden enviar datos");
    Serial.print("   Estado WiFi: ");
    Serial.println(WiFi.status());
    return;
  }
  
  Serial.println("✅ WiFi conectado");
  Serial.print("   IP ESP32: ");
  Serial.println(WiFi.localIP());
  Serial.print("   Señal: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
  
  // Preparar datos
  Serial.println("\n📦 Preparando datos:");
  Serial.print("   device_id: ");
  Serial.println(device_id);
  Serial.print("   flow_rate: ");
  Serial.print(flowRate, 2);
  Serial.println(" L/min");
  Serial.print("   total_volume: ");
  Serial.print(totalVolume, 2);
  Serial.println(" L");
  
  // Crear JSON
  StaticJsonDocument<200> doc;
  doc["device_id"] = device_id;
  doc["flow_rate"] = flowRate;
  doc["total_volume"] = totalVolume;
  
  String jsonData;
  serializeJson(doc, jsonData);
  
  Serial.println("\n📝 JSON generado:");
  Serial.println("   " + jsonData);
  
  // Intentar conexión HTTP
  Serial.println("\n🌐 Conectando al backend:");
  Serial.print("   URL: ");
  Serial.println(backendURL);
  
  HTTPClient http;
  http.begin(backendURL);
  http.addHeader("Content-Type", "application/json");
  
  Serial.println("   Headers configurados");
  Serial.println("   Enviando POST...");
  
  // Enviar POST
  int httpCode = http.POST(jsonData);
  
  Serial.println("\n📡 RESPUESTA DEL SERVIDOR:");
  Serial.print("   Código HTTP: ");
  Serial.println(httpCode);
  
  if (httpCode > 0) {
    Serial.print("   Estado: ");
    if (httpCode == 201) {
      Serial.println("✅ CREADO (201) - Lectura registrada exitosamente");
    } else if (httpCode == 200) {
      Serial.println("✅ OK (200)");
    } else if (httpCode == 400) {
      Serial.println("❌ BAD REQUEST (400) - Datos inválidos");
    } else if (httpCode == 404) {
      Serial.println("❌ NOT FOUND (404) - Endpoint no existe");
    } else if (httpCode == 500) {
      Serial.println("❌ SERVER ERROR (500) - Error en el backend");
    } else {
      Serial.print("⚠️  Código desconocido: ");
      Serial.println(httpCode);
    }
    
    // Mostrar respuesta del servidor
    String response = http.getString();
    if (response.length() > 0) {
      Serial.println("\n   Respuesta del servidor:");
      Serial.println("   " + response);
    }
  } else {
    Serial.println("❌ ERROR EN LA PETICIÓN:");
    Serial.print("   Código de error: ");
    Serial.println(httpCode);
    Serial.print("   Descripción: ");
    Serial.println(http.errorToString(httpCode));
    
    if (httpCode == -1) {
      Serial.println("\n   💡 Posibles causas:");
      Serial.println("      - Backend no está corriendo");
      Serial.println("      - IP incorrecta");
      Serial.println("      - Firewall bloqueando");
      Serial.println("      - ESP32 y PC en redes diferentes");
    }
  }
  
  http.end();
  Serial.println("╚════════════════════════════════════════╝\n");
}

void setup_wifi() {
  Serial.println("\n\n╔════════════════════════════════════════╗");
  Serial.println("║  ESP32 - Control de Válvula + Sensor   ║");
  Serial.println("║           MODO DEBUG ACTIVADO          ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  Serial.print("🔌 Conectando a WiFi: ");
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
    Serial.println("✅ WiFi conectado!");
    Serial.print("📍 IP ESP32: ");
    Serial.println(WiFi.localIP());
    Serial.print("📶 Señal: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    Serial.print("🌐 Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("🔒 Subnet: ");
    Serial.println(WiFi.subnetMask());
  } else {
    Serial.println("❌ Error en conexión WiFi");
    Serial.print("   Estado: ");
    Serial.println(WiFi.status());
  }
}

void setup_gpio() {
  Serial.println("\n⚙️  Configurando GPIO...");
  
  // Configurar válvula
  pinMode(VALVE_PIN, OUTPUT);
  digitalWrite(VALVE_PIN, HIGH);
  Serial.print("   ✅ VALVE_PIN (GPIO17): ");
  Serial.println(digitalRead(VALVE_PIN) == HIGH ? "HIGH - CERRADO" : "LOW - ABIERTO");
  
  // Configurar sensor
  pinMode(SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), pulseCounter, FALLING);
  Serial.println("   ✅ SENSOR_PIN (GPIO35): Interrupción configurada");
  Serial.println("      Modo: FALLING (detecta flanco descendente)");
  Serial.println("      Pull-up: ACTIVADO");
}

void setup_server() {
  Serial.println("\n🌐 Configurando servidor HTTP...");
  
  server.on("/api/valve/open", HTTP_POST, handle_open_valve);
  server.on("/api/valve/close", HTTP_POST, handle_close_valve);
  server.on("/api/status", HTTP_GET, handle_status);
  server.on("/api/valve/status", HTTP_GET, handle_valve_status);
  server.on("/", HTTP_GET, handle_root);
  server.onNotFound(handle_not_found);
  server.begin();
  
  Serial.println("   ✅ Servidor HTTP iniciado en puerto 80");
  Serial.println("   📍 Endpoints disponibles:");
  Serial.println("      POST /api/valve/open");
  Serial.println("      POST /api/valve/close");
  Serial.println("      GET  /api/status");
  Serial.println("      GET  /api/valve/status");
  Serial.println("      GET  /");
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
  
  digitalWrite(VALVE_PIN, LOW);
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
  
  digitalWrite(VALVE_PIN, HIGH);
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
  
  Serial.println("\n═══════════════════════════════════════");
  Serial.println("✅ SISTEMA LISTO!");
  Serial.println("═══════════════════════════════════════");
  Serial.println("💧 Sensor de flujo: ACTIVO");
  Serial.println("🔧 Válvula: LISTA");
  Serial.println("📡 Backend: " + String(backendURL));
  Serial.println("⏱️  Envío automático cada 5 segundos");
  Serial.println("═══════════════════════════════════════\n");
}

void loop() {
  server.handleClient();
  
  // ===== CALCULAR CAUDAL CADA 1 SEGUNDO =====
  if ((millis() - oldTime) > 1000) {
    // Deshabilitar interrupciones temporalmente
    detachInterrupt(digitalPinToInterrupt(SENSOR_PIN));
    
    // ✅ FILTRO DE RUIDO: Ignorar pulsos si son muy pocos (ruido eléctrico)
    // Si hay menos de 10 pulsos por segundo, probablemente es ruido
    if (pulseCount < 10) {
      pulseCount = 0;
      flowRate = 0.0;
    } else {
      // Calcular caudal (L/min) solo si hay suficientes pulsos
      flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
      
      // Calcular volumen acumulado solo si hay flujo real
      float timeElapsed = (millis() - oldTime) / 60000.0;
      totalVolume += flowRate * timeElapsed;
    }
    
    oldTime = millis();
    
    // Mostrar en monitor serial SIEMPRE (incluso sin flujo)
    if (debugMode) {
      Serial.println("─────────────────────────────");
      Serial.print("⏱️  Tiempo: ");
      Serial.print(millis() / 1000);
      Serial.println(" s");
      Serial.print("📈 Pulsos detectados: ");
      Serial.println(pulseCount);
      
      if (pulseCount < 10 && pulseCount > 0) {
        Serial.println("⚠️  RUIDO FILTRADO (< 10 pulsos)");
      }
      
      Serial.print("💧 Caudal: ");
      Serial.print(flowRate, 2);
      Serial.println(" L/min");
      Serial.print("📊 Consumo Total: ");
      Serial.print(totalVolume, 2);
      Serial.println(" L");
      
      if (pulseCount == 0) {
        Serial.println("✅ Sin flujo (correcto si no hay agua)");
      }
    }
    
    pulseCount = 0;
    
    // Reactivar interrupciones
    attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), pulseCounter, FALLING);
    
    // ===== ENVIAR DATOS AL BACKEND CADA 5 SEGUNDOS =====
    // Solo enviar si hay flujo real (flowRate > 0) o cada 30 segundos para mantener conexión
    static unsigned long lastSend = 0;
    if (millis() - lastSend > 5000) {
      // Enviar solo si hay flujo o han pasado 30 segundos
      if (flowRate > 0 || (millis() - lastSend > 30000)) {
        sendDataToBackend();
        lastSend = millis();
      }
    }
  }
  
  delay(10);
}
