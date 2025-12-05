# 🌐 Configuración del ESP32 para Backend Desplegado en Render

## 📋 Cambios Necesarios

Para que tu ESP32 envíe datos a tu backend desplegado en Render en lugar de usar la IP local, necesitas hacer **solo 1 cambio** en tu código Arduino.

---

## ✏️ Cambio Principal: URL del Backend

### ❌ Configuración Actual (Local)

```cpp
const char* backendURL = "http://10.60.136.30:8000/api/sensor-readings/";  // ✅ IP de tu PC
```

### ✅ Nueva Configuración (Producción)

```cpp
const char* backendURL = "https://TU-PROYECTO-BACKEND.onrender.com/api/sensor-readings/";
```

> [!IMPORTANT]
> Reemplaza `TU-PROYECTO-BACKEND` con el nombre real de tu servicio en Render.
> Por ejemplo: `https://water-sensor-backend.onrender.com/api/sensor-readings/`

---

## 🔍 Diferencias Clave

| Aspecto | Local | Producción |
|---------|-------|------------|
| **Protocolo** | `http://` | `https://` |
| **Host** | IP local (ej: `10.60.136.30`) | Dominio de Render (ej: `tu-proyecto.onrender.com`) |
| **Puerto** | `:8000` | No se especifica (usa puerto 443 por defecto) |
| **Endpoint** | `/api/sensor-readings/` | `/api/sensor-readings/` (igual) |

---

## 📝 Código Completo Actualizado

Aquí está la sección de configuración actualizada de tu código:

```cpp
#include <WiFi.h>
#include <WiFiClientSecure.h>  // ← NUEVO: Para HTTPS
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ===== CONFIGURACIÓN WiFi =====
const char* ssid = "OPPO";
const char* password = "jairo233";
const char* device_id = "ESP32_VALVE_001";
const char* device_name = "Válvula Principal";

// ===== CONFIGURACIÓN Backend =====
// PRODUCCIÓN (Render)
const char* backendURL = "https://TU-PROYECTO-BACKEND.onrender.com/api/sensor-readings/";

// DESARROLLO (Local) - Comentar cuando uses producción
// const char* backendURL = "http://10.60.136.30:8000/api/sensor-readings/";
```

---

## 🔐 Configuración HTTPS

Como Render usa HTTPS, necesitas actualizar la función `sendDataToBackend()` para manejar conexiones seguras:

### Opción 1: Sin Verificación de Certificado (Más Simple)

```cpp
void sendDataToBackend() {
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║     INTENTANDO ENVIAR DATOS            ║");
  Serial.println("╚════════════════════════════════════════╝");
  
  // Verificar WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ WiFi desconectado - no se pueden enviar datos");
    return;
  }
  
  Serial.println("✅ WiFi conectado");
  
  // Preparar datos
  StaticJsonDocument<200> doc;
  doc["device_id"] = device_id;
  doc["flow_rate"] = flowRate;
  doc["total_volume"] = totalVolume;
  
  String jsonData;
  serializeJson(doc, jsonData);
  
  Serial.println("\n📝 JSON generado:");
  Serial.println("   " + jsonData);
  
  // Configurar cliente HTTPS
  WiFiClientSecure client;
  client.setInsecure();  // ← No verificar certificado SSL (para desarrollo)
  
  HTTPClient http;
  http.begin(client, backendURL);  // ← Usar cliente seguro
  http.addHeader("Content-Type", "application/json");
  
  Serial.println("   Enviando POST a Render...");
  
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
    } else {
      Serial.print("⚠️  Código: ");
      Serial.println(httpCode);
    }
    
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
  }
  
  http.end();
  Serial.println("╚════════════════════════════════════════╝\n");
}
```

---

## ⚙️ Pasos para Actualizar

1. **Obtén la URL de tu backend en Render:**
   - Ve a tu dashboard de Render
   - Copia la URL de tu Web Service (ej: `https://water-sensor-backend.onrender.com`)

2. **Actualiza el código Arduino:**
   - Cambia `backendURL` con tu URL de Render
   - Asegúrate de incluir `#include <WiFiClientSecure.h>`
   - Actualiza la función `sendDataToBackend()` como se muestra arriba

3. **Sube el código al ESP32:**
   - Conecta tu ESP32 via USB
   - Abre el monitor serial (115200 baud)
   - Sube el código actualizado

4. **Verifica la conexión:**
   - Observa el monitor serial
   - Deberías ver mensajes como:
     ```
     ✅ WiFi conectado
     📝 JSON generado: {"device_id":"ESP32_VALVE_001","flow_rate":5.2,"total_volume":123.5}
     Enviando POST a Render...
     📡 RESPUESTA DEL SERVIDOR:
        Código HTTP: 201
        Estado: ✅ CREADO (201) - Lectura registrada exitosamente
     ```

---

## 🔄 Cambiar entre Local y Producción

Para facilitar el cambio entre desarrollo local y producción, puedes usar esta configuración:

```cpp
// ===== CONFIGURACIÓN Backend =====
#define USE_PRODUCTION true  // ← Cambiar a false para usar local

#if USE_PRODUCTION
  const char* backendURL = "https://TU-PROYECTO-BACKEND.onrender.com/api/sensor-readings/";
#else
  const char* backendURL = "http://10.60.136.30:8000/api/sensor-readings/";
#endif
```

---

## 🐛 Solución de Problemas

### Error: "Connection failed" o código -1

**Causa**: El ESP32 no puede conectarse a Render.

**Soluciones**:
1. Verifica que la URL de Render sea correcta
2. Asegúrate de usar `https://` (no `http://`)
3. Verifica que tu backend en Render esté activo (los servicios gratuitos se duermen después de 15 min de inactividad)

### Error: Código HTTP 403 o 404

**Causa**: El endpoint no existe o está mal configurado.

**Soluciones**:
1. Verifica que el endpoint sea `/api/sensor-readings/` (con la barra final)
2. Prueba el endpoint en tu navegador: `https://TU-PROYECTO.onrender.com/api/sensor-readings/`

### Error: Código HTTP 500

**Causa**: Error en el backend.

**Soluciones**:
1. Revisa los logs de Render (Dashboard → Logs)
2. Verifica que el `device_id` esté registrado en la base de datos

---

## ✅ Checklist de Verificación

- [ ] URL de Render copiada correctamente
- [ ] Código actualizado con `https://` (no `http://`)
- [ ] `#include <WiFiClientSecure.h>` agregado
- [ ] Función `sendDataToBackend()` actualizada
- [ ] Código subido al ESP32
- [ ] Monitor serial muestra conexión exitosa (HTTP 201)
- [ ] Datos visibles en tu frontend/dashboard

---

## 📊 Verificar Datos en el Backend

Para confirmar que los datos están llegando:

1. **Via API REST:**
   ```bash
   curl https://TU-PROYECTO-BACKEND.onrender.com/api/sensor-readings/
   ```

2. **Via navegador:**
   - Abre: `https://TU-PROYECTO-BACKEND.onrender.com/api/sensor-readings/`
   - Deberías ver un JSON con las lecturas

3. **Via frontend:**
   - Accede a tu aplicación web
   - Verifica que los datos del sensor se muestren en tiempo real

---

## 💡 Recomendaciones

1. **Mantén el backend activo**: Los servicios gratuitos de Render se duermen. Considera usar un servicio de "ping" o actualizar a un plan de pago.

2. **Monitorea los logs**: Revisa regularmente los logs de Render para detectar errores.

3. **Configura CORS**: Asegúrate de que tu backend permita peticiones desde el ESP32 (aunque generalmente no es necesario para peticiones POST desde dispositivos IoT).

4. **Usa variables de entorno**: En el futuro, considera usar variables de entorno en Render para configurar parámetros como el `device_id` permitido.

---

¡Listo! Con estos cambios, tu ESP32 enviará datos directamente a tu backend desplegado en Render. 🚀
