#include <WiFi.h>
#include <WebServer.h>

// ===== CONFIGURACIÓN =====
const char* ssid = "OPPO"; // nomrbe de wifi
const char* password = "jairo233"; // contraseña
const char* device_id = "ESP32_VALVE_001"; // crear en base a este nombre en el dashboard de django
const char* device_name = "Válvula Principal"; // nombre al crear el servicio 

// ===== PINES =====
const int VALVE_PIN = 17;  // GPIO17 para controlar válvula

// ===== VARIABLES =====
WebServer server(80);
bool valve_state = false;

void setup_wifi() {
  Serial.println("\n\n╔════════════════════════════════════════╗");
  Serial.println("║  ESP32 - Control de Válvula Solenoide  ║");
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
  pinMode(VALVE_PIN, OUTPUT);
  digitalWrite(VALVE_PIN, HIGH);  // Cerrado por defecto (HIGH = CERRADO)
  
  Serial.println("✓ GPIO configurado:");
  Serial.print("  - VALVE_PIN (GPIO17): ");
  Serial.println(digitalRead(VALVE_PIN) == HIGH ? "HIGH - CERRADO (por defecto)" : "LOW - ABIERTO");
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
  Serial.print("   Estado: ");
  Serial.print(status_str);
  Serial.print(" - GPIO17: ");
  Serial.println(gpio_state == HIGH ? "HIGH" : "LOW");
  
  String response = "{\"device_id\":\"" + String(device_id) + "\",\"device_name\":\"" + String(device_name) + "\",\"valve_state\":\"" + status_str + "\",\"gpio_state\":" + String(gpio_state) + ",\"wifi_signal\":" + String(WiFi.RSSI()) + "}";
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

  Serial.println("✓ Sistema listo! Esperando comandos...\n");
}

void loop() {
  server.handleClient();
  delay(10);
}