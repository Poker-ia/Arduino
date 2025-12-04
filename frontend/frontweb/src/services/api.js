import axios from 'axios';

const API_BASE_URL = 'https://arduino-2a9u.onrender.com/api';

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

  // Obtener estadísticas de consumo
  getStats: (deviceId, startDate, endDate) => api.get('/sensor-readings/stats/', {
    params: { device_id: deviceId, start_date: startDate, end_date: endDate }
  }),

  // Obtener todos los registros
  getAll: () => api.get('/sensor-readings/'),
};

export default api;