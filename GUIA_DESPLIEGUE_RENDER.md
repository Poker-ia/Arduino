# 🚀 Guía de Despliegue del Backend en Render

Esta guía te llevará paso a paso para desplegar tu backend Django en Render.

---

## 📋 Requisitos Previos

- [ ] Cuenta en [Render](https://render.com) (puedes usar tu cuenta de GitHub)
- [ ] Repositorio de GitHub con tu proyecto
- [ ] Tu proyecto Django funcionando localmente

---

## 🔧 Paso 1: Preparar el Proyecto para Producción

### 1.1 Crear archivo `requirements.txt`

En la carpeta raíz de tu backend (`backend/config/`), crea el archivo `requirements.txt` con las dependencias necesarias:

```bash
cd "c:\Users\CHEYLA\Documents\4 CICLO\Arduinos\Proyecto Arduino\backend\config"
pip freeze > requirements.txt
```

O crea manualmente el archivo con estas dependencias mínimas:

```txt
Django>=5.2.8
djangorestframework>=3.14.0
django-cors-headers>=4.3.0
gunicorn>=21.2.0
psycopg2-binary>=2.9.9
python-decouple>=3.8
whitenoise>=6.6.0
dj-database-url>=2.1.0
```

### 1.2 Crear archivo `build.sh`

Crea un archivo llamado `build.sh` en `backend/config/` con el siguiente contenido:

```bash
#!/usr/bin/env bash
# exit on error
set -o errexit

pip install -r requirements.txt

python manage.py collectstatic --no-input
python manage.py migrate
```

> [!IMPORTANT]
> Este script se ejecutará automáticamente cada vez que despliegues en Render.

### 1.3 Actualizar `settings.py` para Producción

Modifica tu archivo `config/settings.py` para que funcione tanto en desarrollo como en producción:

```python
from pathlib import Path
import os
from decouple import config
import dj_database_url

# Build paths inside the project like this: BASE_DIR / 'subdir'.
BASE_DIR = Path(__file__).resolve().parent.parent

# SECURITY WARNING: keep the secret key used in production secret!
SECRET_KEY = config('SECRET_KEY', default='django-insecure-pe6meorta0(xts1r_#d=#b@=q0frf+%4@ila8$-sn22*)7dag-')

# SECURITY WARNING: don't run with debug turned on in production!
DEBUG = config('DEBUG', default=False, cast=bool)

ALLOWED_HOSTS = config('ALLOWED_HOSTS', default='localhost,127.0.0.1').split(',')

# Si estamos en Render, agregar el host de Render
RENDER_EXTERNAL_HOSTNAME = os.environ.get('RENDER_EXTERNAL_HOSTNAME')
if RENDER_EXTERNAL_HOSTNAME:
    ALLOWED_HOSTS.append(RENDER_EXTERNAL_HOSTNAME)

# Application definition
INSTALLED_APPS = [
    'django.contrib.admin',
    'django.contrib.auth',
    'django.contrib.contenttypes',
    'django.contrib.sessions',
    'django.contrib.messages',
    'django.contrib.staticfiles',
    'rest_framework',
    'corsheaders',
    'api',
]

MIDDLEWARE = [
    'django.middleware.security.SecurityMiddleware',
    'whitenoise.middleware.WhiteNoiseMiddleware',  # ← Agregar WhiteNoise
    'corsheaders.middleware.CorsMiddleware',
    'django.contrib.sessions.middleware.SessionMiddleware',
    'django.middleware.common.CommonMiddleware',
    'django.middleware.csrf.CsrfViewMiddleware',
    'django.contrib.auth.middleware.AuthenticationMiddleware',
    'django.contrib.messages.middleware.MessageMiddleware',
    'django.middleware.clickjacking.XFrameOptionsMiddleware',
]

# CORS Configuration
CORS_ALLOWED_ORIGINS = [
    "http://localhost:3000",
    "http://localhost:5173",
    "http://127.0.0.1:3000",
    "http://127.0.0.1:5173",
    "http://localhost:8000",
]

# Agregar tu dominio de frontend cuando lo despliegues
FRONTEND_URL = config('FRONTEND_URL', default='')
if FRONTEND_URL:
    CORS_ALLOWED_ORIGINS.append(FRONTEND_URL)

CORS_ALLOW_CREDENTIALS = True

# Database
# https://docs.djangoproject.com/en/5.2/ref/settings/#databases

DATABASES = {
    'default': dj_database_url.config(
        default=f'sqlite:///{BASE_DIR / "db.sqlite3"}',
        conn_max_age=600
    )
}

# Static files (CSS, JavaScript, Images)
STATIC_URL = '/static/'
STATIC_ROOT = os.path.join(BASE_DIR, 'staticfiles')
STATICFILES_STORAGE = 'whitenoise.storage.CompressedManifestStaticFilesStorage'

# Default primary key field type
DEFAULT_AUTO_FIELD = 'django.db.models.BigAutoField'

REST_FRAMEWORK = {
    'DEFAULT_PAGINATION_CLASS': 'rest_framework.pagination.PageNumberPagination',
    'PAGE_SIZE': 100
}
```

### 1.4 Actualizar `.env` (solo para desarrollo local)

Tu archivo `.env` debe verse así:

```env
SECRET_KEY=tu-clave-secreta-super-segura
DEBUG=True
ALLOWED_HOSTS=localhost,127.0.0.1
DATABASE_URL=sqlite:///db.sqlite3
```

> [!WARNING]
> **NUNCA** subas el archivo `.env` a GitHub. Asegúrate de que esté en tu `.gitignore`.

---

## 📦 Paso 2: Subir el Proyecto a GitHub

### 2.1 Crear `.gitignore`

Crea o actualiza tu archivo `.gitignore` en la raíz del proyecto:

```gitignore
# Python
*.pyc
__pycache__/
*.py[cod]
*$py.class
*.so
.Python
venv/
env/
ENV/

# Django
*.log
db.sqlite3
db.sqlite3-journal
/staticfiles/
/media/

# Environment variables
.env
.env.local

# IDEs
.vscode/
.idea/
*.swp
*.swo
*~

# OS
.DS_Store
Thumbs.db
```

### 2.2 Inicializar Git y Subir a GitHub

```bash
# Navegar a la carpeta del proyecto
cd "c:\Users\CHEYLA\Documents\4 CICLO\Arduinos\Proyecto Arduino"

# Inicializar git (si no lo has hecho)
git init

# Agregar todos los archivos
git add .

# Hacer commit
git commit -m "Preparar proyecto para despliegue en Render"

# Conectar con tu repositorio de GitHub
git remote add origin https://github.com/TU_USUARIO/TU_REPOSITORIO.git

# Subir los cambios
git push -u origin main
```

---

## 🌐 Paso 3: Crear el Servicio Web en Render

### 3.1 Acceder a Render

1. Ve a [https://render.com](https://render.com)
2. Inicia sesión con tu cuenta de GitHub
3. Haz clic en **"New +"** → **"Web Service"**

### 3.2 Conectar tu Repositorio

1. Busca y selecciona tu repositorio de GitHub
2. Haz clic en **"Connect"**

### 3.3 Configurar el Servicio

Completa los siguientes campos:

| Campo | Valor |
|-------|-------|
| **Name** | `tu-proyecto-backend` (o el nombre que prefieras) |
| **Region** | Selecciona la región más cercana (ej: `Oregon (US West)`) |
| **Branch** | `main` (o la rama que uses) |
| **Root Directory** | `backend/config` |
| **Runtime** | `Python 3` |
| **Build Command** | `./build.sh` |
| **Start Command** | `gunicorn config.wsgi:application` |
| **Instance Type** | `Free` (para empezar) |

### 3.4 Agregar Variables de Entorno

En la sección **"Environment Variables"**, agrega las siguientes variables:

| Key | Value |
|-----|-------|
| `SECRET_KEY` | `tu-clave-secreta-super-segura-y-larga-para-produccion` |
| `DEBUG` | `False` |
| `PYTHON_VERSION` | `3.11.0` (o tu versión de Python) |
| `DATABASE_URL` | _(Render lo creará automáticamente si usas PostgreSQL)_ |

> [!TIP]
> Para generar una SECRET_KEY segura, puedes usar:
> ```python
> python -c "from django.core.management.utils import get_random_secret_key; print(get_random_secret_key())"
> ```

### 3.5 Crear el Servicio

1. Haz clic en **"Create Web Service"**
2. Render comenzará a construir y desplegar tu aplicación
3. Espera a que el despliegue se complete (puede tomar 5-10 minutos)

---

## 🗄️ Paso 4: Configurar Base de Datos PostgreSQL (Opcional pero Recomendado)

### 4.1 Crear Base de Datos PostgreSQL

1. En el dashboard de Render, haz clic en **"New +"** → **"PostgreSQL"**
2. Completa los campos:
   - **Name**: `tu-proyecto-db`
   - **Database**: `tu_proyecto_db`
   - **User**: (se genera automáticamente)
   - **Region**: La misma que tu web service
   - **PostgreSQL Version**: `16`
   - **Instance Type**: `Free`
3. Haz clic en **"Create Database"**

### 4.2 Conectar la Base de Datos al Web Service

1. Ve a tu **Web Service** en Render
2. Ve a la pestaña **"Environment"**
3. Copia la **Internal Database URL** de tu base de datos PostgreSQL
4. Agrega/actualiza la variable de entorno:
   - **Key**: `DATABASE_URL`
   - **Value**: _(pega la URL interna de la base de datos)_
5. Guarda los cambios

> [!NOTE]
> Render redesplegará automáticamente tu aplicación cuando cambies las variables de entorno.

---

## ✅ Paso 5: Verificar el Despliegue

### 5.1 Revisar los Logs

1. En tu Web Service, ve a la pestaña **"Logs"**
2. Verifica que no haya errores
3. Deberías ver algo como:
   ```
   ==> Build successful 🎉
   ==> Deploying...
   ==> Starting service...
   ```

### 5.2 Probar la API

1. Copia la URL de tu servicio (ej: `https://tu-proyecto-backend.onrender.com`)
2. Abre tu navegador y ve a: `https://tu-proyecto-backend.onrender.com/api/`
3. Deberías ver la página de la API de Django REST Framework

### 5.3 Probar Endpoints Específicos

Prueba tus endpoints con herramientas como:
- **Postman**
- **Thunder Client** (extensión de VS Code)
- **curl**

Ejemplo con curl:
```bash
curl https://tu-proyecto-backend.onrender.com/api/devices/
```

---

## 🔄 Paso 6: Actualizar el Frontend

### 6.1 Actualizar la URL de la API

En tu proyecto frontend, actualiza la URL base de la API para que apunte a tu backend en Render:

```javascript
// Ejemplo en tu archivo de configuración de API
const API_BASE_URL = process.env.NODE_ENV === 'production' 
  ? 'https://tu-proyecto-backend.onrender.com'
  : 'http://localhost:8000';
```

### 6.2 Actualizar CORS en el Backend

1. Ve a Render → Tu Web Service → Environment
2. Agrega la variable `FRONTEND_URL` con la URL de tu frontend desplegado
3. Ejemplo: `https://tu-frontend.vercel.app`

---

## 🐛 Solución de Problemas Comunes

### Error: "Application failed to respond"

**Causa**: El comando de inicio está mal configurado.

**Solución**: Verifica que el Start Command sea:
```bash
gunicorn config.wsgi:application
```

### Error: "ModuleNotFoundError"

**Causa**: Falta una dependencia en `requirements.txt`.

**Solución**: 
1. Agrega la dependencia faltante a `requirements.txt`
2. Haz commit y push a GitHub
3. Render redesplegará automáticamente

### Error: "DisallowedHost"

**Causa**: El dominio de Render no está en `ALLOWED_HOSTS`.

**Solución**: Verifica que tengas esto en `settings.py`:
```python
RENDER_EXTERNAL_HOSTNAME = os.environ.get('RENDER_EXTERNAL_HOSTNAME')
if RENDER_EXTERNAL_HOSTNAME:
    ALLOWED_HOSTS.append(RENDER_EXTERNAL_HOSTNAME)
```

### La aplicación es lenta al inicio

**Causa**: Los servicios gratuitos de Render se "duermen" después de 15 minutos de inactividad.

**Solución**: 
- Actualiza a un plan de pago, o
- Usa un servicio de "ping" para mantener tu app activa

---

## 🎯 Configuración del ESP32

Una vez desplegado el backend, actualiza tu código Arduino para que apunte a la URL de Render:

```cpp
const char* serverName = "https://tu-proyecto-backend.onrender.com/api/sensor-data/";
```

> [!CAUTION]
> Asegúrate de que tu ESP32 soporte HTTPS. Si no, considera usar un proxy HTTP o actualizar el firmware.

---

## 📚 Recursos Adicionales

- [Documentación oficial de Render](https://render.com/docs)
- [Deploy Django on Render](https://render.com/docs/deploy-django)
- [Django Deployment Checklist](https://docs.djangoproject.com/en/5.2/howto/deployment/checklist/)

---

## ✨ Checklist Final

- [ ] `requirements.txt` creado con todas las dependencias
- [ ] `build.sh` creado y con permisos de ejecución
- [ ] `settings.py` actualizado para producción
- [ ] `.gitignore` configurado correctamente
- [ ] Proyecto subido a GitHub
- [ ] Web Service creado en Render
- [ ] Variables de entorno configuradas
- [ ] Base de datos PostgreSQL creada (opcional)
- [ ] Despliegue exitoso verificado
- [ ] Frontend actualizado con la nueva URL
- [ ] ESP32 configurado con la URL de producción

---

¡Felicidades! 🎉 Tu backend Django está ahora desplegado en Render y listo para recibir datos de tu ESP32.
