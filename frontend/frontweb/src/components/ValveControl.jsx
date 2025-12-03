import React, { useState } from 'react';
import { deviceAPI } from '../services/api';
import './ValveControl.css';

function ValveControl({ device, onStatusChange }) {
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState(null);
  const [valveStatus, setValveStatus] = useState('closed');

  const handleOpenValve = async () => {
    setLoading(true);
    setError(null);
    try {
      await deviceAPI.openValve(device.id);
      setValveStatus('open');
      onStatusChange('open');
    } catch (err) {
      setError(err.response?.data?.error || 'Error al abrir válvula');
    } finally {
      setLoading(false);
    }
  };

  const handleCloseValve = async () => {
    setLoading(true);
    setError(null);
    try {
      await deviceAPI.closeValve(device.id);
      setValveStatus('closed');
      onStatusChange('closed');
    } catch (err) {
      setError(err.response?.data?.error || 'Error al cerrar válvula');
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

      <div className="button-group">
        <button
          className="btn btn-open"
          onClick={handleOpenValve}
          disabled={loading || !device.is_online}
        >
          {loading ? 'Procesando...' : 'ABRIR'}
        </button>
        <button
          className="btn btn-close"
          onClick={handleCloseValve}
          disabled={loading || !device.is_online}
        >
          {loading ? 'Procesando...' : 'CERRAR'}
        </button>
      </div>

      {!device.is_online && (
        <p className="offline-warning">Dispositivo desconectado</p>
      )}
    </div>
  );
}

export default ValveControl;