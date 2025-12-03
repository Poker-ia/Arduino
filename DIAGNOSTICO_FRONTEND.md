# 🔍 Diagnóstico: Frontend No Se Actualiza

## Problema Encontrado

El frontend muestra **datos antiguos** (lectura de prueba del 16:31) porque:

❌ **El ESP32 NO está enviando datos nuevos al backend**

## Verificación

```bash
curl "http://localhost:8000/api/sensor-readings/"
```

**Resultado**: Solo 1 lectura (la de prueba)
- No hay lecturas nuevas del ESP32
- El ESP32 probablemente está mostrando "connection refused" en el Monitor Serial

## Causas Posibles

1. **Firewall de Windows bloqueando** el puerto 8000
2. **Backend corriendo en 127.0.0.1** (solo localhost) en lugar de 0.0.0.0 (todas las interfaces)
3. **ESP32 y PC en subredes diferentes** (aunque ambos están en 10.60.136.X)

## Solución

### Opción 1: Configurar Django para escuchar en todas las interfaces

Detén el servidor Django (Ctrl+C) y reinícialo con:

```bash
python manage.py runserver 0.0.0.0:8000
```

Esto permite que el ESP32 se conecte desde la red local.

### Opción 2: Verificar Firewall

Ejecuta en PowerShell (como Administrador):

```powershell
New-NetFirewallRule -DisplayName "Django Dev Server" -Direction Inbound -LocalPort 8000 -Protocol TCP -Action Allow
```

### Opción 3: Verificar en Monitor Serial

Abre el Monitor Serial del ESP32 y busca:

**Si ves:**
```
❌ ERROR EN LA PETICIÓN:
   Código de error: -1
   Descripción: connection refused
```

**Significa**: El backend no está accesible desde la red

**Si ves:**
```
✅ CREADO (201) - Lectura registrada exitosamente
```

**Significa**: Todo funciona, solo necesitas refrescar el frontend

## Próximos Pasos

1. Reinicia Django con `python manage.py runserver 0.0.0.0:8000`
2. Verifica en Monitor Serial que dice "✅ CREADO (201)"
3. Refresca el frontend (F5)
4. Los datos deberían actualizarse cada 5 segundos
