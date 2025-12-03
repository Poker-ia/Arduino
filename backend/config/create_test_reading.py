import requests
import json

# Crear una lectura de prueba
url = "http://localhost:8000/api/sensor-readings/"
data = {
    "device": 1,
    "flow_rate": 5.2,
    "total_volume": 100.5
}

response = requests.post(url, json=data)

if response.status_code == 201:
    print("✓ Lectura de prueba creada exitosamente!")
    print(json.dumps(response.json(), indent=2))
else:
    print(f"✗ Error: {response.status_code}")
    print(response.text)
