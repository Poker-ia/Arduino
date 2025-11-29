import axios from 'axios';

const API_BASE_URL = 'http://localhost:8000/api';

const api = axios.create({
  baseURL: API_BASE_URL,
  headers: {
    'Content-Type': 'application/json',
  },
});

export const deviceAPI = {
  // Obtener todos los dispositivos
  getAll: () => api.get('/devices/'),

  // Obtener dispositivo por ID
  getById: (id) => api.get(`/devices/${id}/`),

  // Abrir válvula
  openValve: (id) => api.post(`/devices/${id}/open_valve/`),

  // Cerrar válvula
  closeValve: (id) => api.post(`/devices/${id}/close_valve/`),

  // Obtener estado del dispositivo
  getStatus: (id) => api.get(`/devices/${id}/status/`),
};

export const valveControlAPI = {
  // Obtener historial de válvula
  getHistory: (deviceId) => api.get(`/valve-controls/by_device/?device_id=${deviceId}`),

  // Obtener todos los registros
  getAll: () => api.get('/valve-controls/'),
};

export const sensorAPI = {
  // Obtener última lectura del sensor
  getLatest: (deviceId) => api.get(`/sensor-readings/latest/?device_id=${deviceId}`),

  // Obtener todos los registros
  getAll: () => api.get('/sensor-readings/'),
};

export default api;