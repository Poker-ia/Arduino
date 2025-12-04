import React, { useState, useEffect } from 'react';
import './WaterSensorDisplay.css';
import { sensorAPI } from '../services/api';

function WaterSensorDisplay({ device }) {
  const [sensorData, setSensorData] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);
  const [lastUpdate, setLastUpdate] = useState(null);

  useEffect(() => {
    fetchSensorData();
    // Actualizar cada 5 segundos
    const interval = setInterval(fetchSensorData, 5000);
    return () => clearInterval(interval);
  }, [device.id]);



  const fetchSensorData = async () => {
    try {
      const response = await sensorAPI.getLatest(device.id);

      if (response.data) {
        setSensorData(response.data);
        setLastUpdate(new Date());
        setError(null);
      } else {
        setError('Error al obtener datos del sensor');
      }
    } catch (err) {
      console.error('Error fetching sensor data:', err);
      if (err.response && err.response.status === 404) {
        // No hay lecturas todavía - esto es normal al inicio
        setError(null);
        setSensorData(null);
      } else {
        setError(null); // No mostrar error si el backend no responde
      }
    } finally {
      setLoading(false);
    }
  };

  const getTimeSinceUpdate = () => {
    if (!lastUpdate) return '';
    const seconds = Math.floor((new Date() - lastUpdate) / 1000);
    if (seconds < 60) return `hace ${seconds}s`;
    const minutes = Math.floor(seconds / 60);
    return `hace ${minutes}m`;
  };

  if (loading) {
    return (
      <div className="water-sensor-display loading">
        <div className="sensor-spinner"></div>
        <p>Cargando datos del sensor...</p>
      </div>
    );
  }

  // Si no hay datos, mostrar estado de espera
  if (!sensorData) {
    return (
      <div className="water-sensor-display waiting">
        <div className="sensor-header">
          <h3>Sensor de Agua</h3>
        </div>
        <div className="waiting-message">
          <p>Esperando datos del sensor...</p>
          <small>Asegúrate de que el ESP32 esté conectado y enviando datos</small>
        </div>
      </div>
    );
  }

  return (
    <div className="water-sensor-display">
      <div className="sensor-header">
        <h3>Sensor de Agua</h3>
        {lastUpdate && (
          <span className="last-update">{getTimeSinceUpdate()}</span>
        )}
      </div>

      <div className="sensor-metrics">
        <div className="metric">
          <div className="metric-label">Caudal Actual</div>
          <div className="metric-value">
            {sensorData.flow_rate.toFixed(2)}
            <span className="metric-unit">L/min</span>
          </div>
        </div>

        <div className="metric">
          <div className="metric-label">Consumo Total</div>
          <div className="metric-value">
            {sensorData.total_volume.toFixed(1)}
            <span className="metric-unit">L</span>
          </div>
        </div>
      </div>

      <div className="sensor-footer">
        <small>
          Última lectura: {new Date(sensorData.timestamp).toLocaleString('es-ES')}
        </small>
      </div>
    </div>
  );
}

export default WaterSensorDisplay;
