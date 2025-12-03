import React, { useState } from 'react';
import { deviceAPI } from '../services/api';
import './ValveControl.css';

function ValveControl({ device, onStatusChange }) {
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState(null);
  const [valveStatus, setValveStatus] = useState('closed');

  const handleToggle = async () => {
    if (loading || !device.is_online) return;

    setLoading(true);
    setError(null);

    const newStatus = valveStatus === 'closed' ? 'open' : 'closed';

    try {
      if (newStatus === 'open') {
        await deviceAPI.openValve(device.id);
      } else {
        await deviceAPI.closeValve(device.id);
      }
      setValveStatus(newStatus);
      onStatusChange(newStatus);
    } catch (err) {
      setError(err.response?.data?.error || 'Error al cambiar estado de válvula');
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="valve-control">
      <div className="valve-status">
        <h3>Estado de Válvula</h3>
        <p className={`status ${valveStatus}`}>
          {valveStatus === 'open' ? 'ABIERTA' : 'CERRADA'}
        </p>
      </div>

      {error && <div className="error-message">{error}</div>}

      <div className="toggle-container">
        <button
          className={`toggle-switch ${valveStatus === 'open' ? 'active' : ''} ${loading ? 'loading' : ''}`}
          onClick={handleToggle}
          disabled={loading || !device.is_online}
          aria-label={valveStatus === 'open' ? 'Cerrar válvula' : 'Abrir válvula'}
        >
          <div className="toggle-knob">
            <span className="toggle-icon">
              {valveStatus === 'open' ? '💧' : '🔒'}
            </span>
          </div>
        </button>
        <span className="toggle-label">
          {loading ? 'Procesando...' : 'Toca para cambiar'}
        </span>
      </div>

      {!device.is_online && (
        <p className="offline-warning">Dispositivo desconectado</p>
      )}
    </div>
  );
}

export default ValveControl;