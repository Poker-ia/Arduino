import React, { useState, useEffect } from 'react';
import { deviceAPI } from '../services/api';
import DeviceCard from './DeviceCard';
import './Dashboard.css';

function Dashboard() {
  const [devices, setDevices] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  useEffect(() => {
    fetchDevices();
    // Recargar dispositivos cada 10 segundos
    const interval = setInterval(fetchDevices, 10000);
    return () => clearInterval(interval);
  }, []);

  const fetchDevices = async () => {
    try {
      const response = await deviceAPI.getAll();
      
      console.log('Respuesta completa:', response);
      console.log('Datos recibidos:', response.data);
      
      // ✅ ARREGLO: Maneja tanto arrays como objetos con propiedad 'results'
      let devicesData = [];
      
      if (Array.isArray(response.data)) {
        // Si es un array directo
        devicesData = response.data;
      } else if (response.data && Array.isArray(response.data.results)) {
        // Si Django devuelve { results: [...] }
        devicesData = response.data.results;
      } else if (response.data && typeof response.data === 'object') {
        // Si es un objeto, intenta obtener los valores
        devicesData = Object.values(response.data);
      }
      
      // Verifica que sea un array antes de usarlo
      if (!Array.isArray(devicesData)) {
        console.warn('Los datos no son un array:', devicesData);
        devicesData = [];
      }
      
      setDevices(devicesData);
      setError(null);
    } catch (err) {
      console.error('Error completo:', err);
      setError('Error al cargar dispositivos: ' + (err.response?.data?.detail || err.message));
      setDevices([]); // ✅ Asegura que sea array aunque haya error
    } finally {
      setLoading(false);
    }
  };

  if (loading) {
    return (
      <div className="dashboard loading">
        <div className="spinner"></div>
        <p>Cargando dispositivos...</p>
      </div>
    );
  }

  if (error) {
    return (
      <div className="dashboard error">
        <h2>⚠️ Error</h2>
        <p>{error}</p>
        <button onClick={fetchDevices} className="retry-btn">
          Reintentar
        </button>
      </div>
    );
  }

  if (!devices || devices.length === 0) {
    return (
      <div className="dashboard empty">
        <h2>📭 No hay dispositivos registrados</h2>
        <p>Registra un ESP32 en la panel de administración de Django para comenzar</p>
        <a href="http://localhost:8000/admin" target="_blank" rel="noopener noreferrer" className="admin-link">
          Ir a Admin Django
        </a>
      </div>
    );
  }

  return (
    <div className="dashboard">
      <div className="dashboard-header">
        <h1>🚰 Sistema de Control de Válvula</h1>
        <p>Controla tus dispositivos ESP32 de forma remota</p>
        <p className="device-count">Dispositivos conectados: {devices.length}</p>
      </div>

      <div className="devices-grid">
        {devices.map((device) => (
          <DeviceCard key={device.id} device={device} />
        ))}
      </div>
    </div>
  );
}

export default Dashboard;