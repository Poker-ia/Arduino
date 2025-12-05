# 🚀 Guía de Implementación - Control Remoto ESP32

## ✅ Cambios Completados

### 1. Backend (Django) - ✅ LISTO

#### Archivos Modificados:
- ✅ `api/models.py` - Agregados campos `desired_valve_state` y `current_valve_state`
- ✅ `api/views.py` - Implementada arquitectura de polling
- ✅ Migración creada y aplicada

#### Nuevos Endpoints:

**Para el Frontend:**
- `POST /api/devices/{id}/open_valve/` - Solicitar apertura
- `POST /api/devices/{id}/close_valve/` - Solicitar cierre
- `GET /api/devices/{id}/status/` - Obtener estado completo

**Para el ESP32:**
- `GET /api/devices/get_pending_command/?device_id=ESP32_VALVE_001` - Consultar comandos
- `POST /api/devices/report_valve_state/` - Reportar estado

### 2. ESP32 (Arduino) - ✅ LISTO

#### Archivo Actualizado:
- ✅ `esp32_water_sensor_production.ino`

#### Nuevas Funciones:
- ✅ `checkPendingCommands()` - Consulta backend cada 5s
- ✅ `reportValveState(String state)` - Reporta estado al backend

---

## 📝 Pasos para Desplegar

### Paso 1: Actualizar Backend en Render

```bash
# En tu carpeta del proyecto
cd "c:\Users\CHEYLA\Documents\4 CICLO\Arduinos\Proyecto Arduino"

# Hacer commit de los cambios
git add backend/config/api/models.py
git add backend/config/api/views.py
git add backend/config/api/migrations/

git commit -m "feat: implementar arquitectura de polling para control remoto de válvula"

# Subir a GitHub
git push origin main
```

Render detectará los cambios y redesplegará automáticamente.

### Paso 2: Configurar ESP32

1. **Abre el archivo**: `esp32_water_sensor_production.ino`

2. **Actualiza la línea 16** con tu URL de Render:
   ```cpp
   const char* backendURL = "https://TU-PROYECTO-BACKEND.onrender.com";
   ```
   
   Por ejemplo:
   ```cpp
   const char* backendURL = "https://water-sensor-api.onrender.com";
   ```

3. **Sube el código al ESP32**:
   - Conecta el ESP32 via USB
   - Abre Arduino IDE
   - Selecciona el puerto correcto
   - Click en "Subir"

4. **Abre el Monitor Serial** (115200 baud) y verifica:
   ```
   ✅ WiFi conectado
   📡 Backend: https://tu-backend.onrender.com
   🔄 Polling de comandos: cada 5 segundos
   ```

### Paso 3: Actualizar Frontend

Necesitas actualizar las llamadas API en tu frontend. Busca donde se hace la petición para abrir/cerrar válvula y cambia:

**❌ Antes:**
```javascript
// Llamada directa al ESP32 (no funciona con backend en Render)
await fetch(`http://${deviceIP}/api/valve/open`, { 
  method: 'POST' 
});
```

**✅ Ahora:**
```javascript
// Llamada al backend (funciona con Render)
await fetch(`https://tu-backend.onrender.com/api/devices/${deviceId}/open_valve/`, {
  method: 'POST',
  headers: {
    'Content-Type': 'application/json'
  }
});
```

---

## 🔄 Cómo Funciona

### Flujo de Control de Válvula:

```
1. Usuario hace click en "Abrir Válvula" en el frontend
   ↓
2. Frontend envía POST a backend: /api/devices/1/open_valve/
   ↓
3. Backend guarda comando en base de datos (desired_valve_state = 'open')
   ↓
4. Backend responde: "Comando registrado"
   ↓
5. ESP32 consulta cada 5s: GET /api/devices/get_pending_command/
   ↓
6. Backend responde: {"command": "open"}
   ↓
7. ESP32 ejecuta: digitalWrite(VALVE_PIN, LOW)
   ↓
8. ESP32 reporta: POST /api/devices/report_valve_state/ {"valve_state": "open"}
   ↓
9. Backend actualiza current_valve_state y registra en historial
   ↓
10. Frontend consulta estado y muestra "Válvula Abierta"
```

**⏱️ Delay máximo**: 5 segundos (tiempo de polling)

---

## 🧪 Pruebas

### Probar Backend Localmente

```bash
cd backend/config
python manage.py runserver
```

Luego prueba los endpoints:

```bash
# Solicitar abrir válvula
curl -X POST http://localhost:8000/api/devices/1/open_valve/

# ESP32 consulta comandos
curl http://localhost:8000/api/devices/get_pending_command/?device_id=ESP32_VALVE_001

# ESP32 reporta estado
curl -X POST http://localhost:8000/api/devices/report_valve_state/ \
  -H "Content-Type: application/json" \
  -d '{"device_id":"ESP32_VALVE_001","valve_state":"open"}'
```

### Probar ESP32

1. Abre el Monitor Serial (115200 baud)
2. Deberías ver cada 5 segundos:
   ```
   🔍 Consultando comandos pendientes...
   📥 Respuesta: {"command":"none",...}
   ✅ Sin comandos pendientes
   ```

3. Desde el frontend, solicita abrir la válvula
4. En el siguiente ciclo (máx 5s) deberías ver:
   ```
   🔍 Consultando comandos pendientes...
   📥 Respuesta: {"command":"open",...}
   🔓 Ejecutando: ABRIR VÁLVULA
   📤 Reportando estado de válvula: open
   ✅ Estado reportado exitosamente
   ```

---

## ⚠️ Notas Importantes

### Sobre el Consumo del ESP32

**¿Se calentará el ESP32?** ❌ NO

- El ESP32 ya envía datos cada 5 segundos
- Agregar una petición GET adicional es mínimo (~100ms de trabajo)
- Consumo total: ~6% del tiempo
- Temperatura: Normal (40-60°C)

### Sobre el Delay

- **Delay máximo**: 5 segundos entre comando y ejecución
- Si necesitas tiempo real, considera MQTT (más complejo)
- Para control de válvulas, 5 segundos es aceptable

### Sobre Render (Plan Gratuito)

- Los servicios gratuitos se "duermen" después de 15 min de inactividad
- Primera petición puede tardar 30-60 segundos (mientras "despierta")
- Solución: Usar un servicio de "ping" o actualizar a plan de pago

---

## 🐛 Solución de Problemas

### ESP32 no recibe comandos

**Síntomas**: Monitor serial muestra "Sin comandos pendientes" siempre

**Soluciones**:
1. Verifica que la URL del backend sea correcta
2. Verifica que el `device_id` coincida con el de la base de datos
3. Revisa los logs de Render para ver si llegan las peticiones

### Backend responde 404

**Síntomas**: ESP32 muestra "Error HTTP: 404"

**Soluciones**:
1. Verifica que la migración se haya aplicado en Render
2. Verifica que la URL incluya el trailing slash: `/api/devices/get_pending_command/`
3. Revisa que el endpoint esté registrado en las rutas

### Válvula no se activa

**Síntomas**: Backend registra comando pero válvula no responde

**Soluciones**:
1. Verifica conexión del relé (GPIO17)
2. Revisa el monitor serial para ver si el ESP32 recibe el comando
3. Verifica que el ESP32 esté online (`is_online = True` en base de datos)

---

## ✅ Checklist Final

- [ ] Backend actualizado y desplegado en Render
- [ ] Migración aplicada en Render
- [ ] ESP32 actualizado con URL de Render
- [ ] ESP32 muestra "Polling de comandos: cada 5 segundos"
- [ ] Frontend actualizado para usar endpoints del backend
- [ ] Prueba: Abrir válvula desde frontend funciona
- [ ] Prueba: Cerrar válvula desde frontend funciona
- [ ] Prueba: Estado se sincroniza correctamente

---

¡Listo! Tu sistema ahora funciona completamente en la nube. 🎉
