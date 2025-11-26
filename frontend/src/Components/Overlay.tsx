import React from 'react';
import './Overlay.css';

export default function Overlay({ onClose }: { onClose: () => void }) {
  const items = ['A', 'B', 'C', 'D', 'E', 'F'];
  return (
    <div className="overlay" onClick={onClose}>
      {items.map((item, idx) => (
        <div key={idx} className="overlay-item">
          {item}
        </div>
      ))}
    </div>
  );
}