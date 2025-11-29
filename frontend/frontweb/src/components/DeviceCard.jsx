import React, { useState } from 'react';
import ValveControl from './ValveControl';
import './DeviceCard.css';

function DeviceCard({ device }) {
  const [lastControl, setLastControl] = useState(null);

  const handleStatusChange = (status) => {
    setLastControl(status);
  };

  return (
    <div className="device-card">
      <div className="card-header">
        <h2>{device.name}</h2>
        <span className={`status-badge ${device.is_online ? 'online' : 'offline'}`}>
          {device.is_online ? '🟢 En línea' : '🔴 Sin conexión'}
        </span>
      </div>

      <div className="card-body">
        <div className="info-group">
          <label>ID del Dispositivo:</label>
          <span>{device.device_id}</span>
        </div>
        <div className="info-group">
          <label>Dirección IP:</label>
          <span>{device.ip_address}</span>
        </div>
        <div className="info-group">
          <label>Registrado:</label>
          <span>{new Date(device.created_at).toLocaleDateString()}</span>
        </div>
      </div>

      <div className="card-controls">
        <ValveControl device={device} onStatusChange={handleStatusChange} />
      </div>

      {lastControl && (
        <div className="last-control-info">
          Última acción: {lastControl === 'open' ? 'Válvula abierta' : 'Válvula cerrada'}
        </div>
      )}
    </div>
  );
}

export default DeviceCard;