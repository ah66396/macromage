// frontend/src/App.tsx
import './App.css';
import React, { useEffect, useRef, useState } from 'react';

const TILE_SIZE = 200;
const BUFFER_TILES = 2;
const MIN_SCALE = 0.2;
const MAX_SCALE = 4;

export default function App() {
  const containerRef = useRef<HTMLDivElement | null>(null);

  const [grid, setGrid] = useState({ x: 0, y: 0 });
  const gridRef = useRef(grid);
  const [scale, setScale] = useState(1);
  const scaleRef = useRef(scale);

  const draggingRef = useRef(false);
  const lastPointerRef = useRef({ x: 0, y: 0 });

  const [viewport, setViewport] = useState({ w: window.innerWidth, h: window.innerHeight });

  useEffect(() => {
    const onResize = () => setViewport({ w: window.innerWidth, h: window.innerHeight });
    window.addEventListener('resize', onResize);
    return () => window.removeEventListener('resize', onResize);
  }, []);

  useEffect(() => {
    gridRef.current = grid;
  }, [grid]);

  useEffect(() => {
    scaleRef.current = scale;
  }, [scale]);

  useEffect(() => {
    let animationFrame: number;

    const updateGrid = () => {
      setGrid({ ...gridRef.current });
    };

    const handleMove = (clientX: number, clientY: number) => {
      if (!draggingRef.current) return;
      const dx = clientX - lastPointerRef.current.x;
      const dy = clientY - lastPointerRef.current.y;
      gridRef.current = {
        x: gridRef.current.x + dx,
        y: gridRef.current.y + dy,
      };
      lastPointerRef.current = { x: clientX, y: clientY };
      cancelAnimationFrame(animationFrame);
      animationFrame = requestAnimationFrame(updateGrid);
    };

    const handleMouseMove = (e: MouseEvent) => handleMove(e.clientX, e.clientY);
    const handleTouchMove = (e: TouchEvent) => {
      if (e.touches.length === 0) return;
      handleMove(e.touches[0].clientX, e.touches[0].clientY);
    };

    window.addEventListener('mousemove', handleMouseMove);
    window.addEventListener('touchmove', handleTouchMove);

    return () => {
      window.removeEventListener('mousemove', handleMouseMove);
      window.removeEventListener('touchmove', handleTouchMove);
      cancelAnimationFrame(animationFrame);
    };
  }, []);

  const handleMouseDown = (clientX: number, clientY: number) => {
    draggingRef.current = true;
    lastPointerRef.current = { x: clientX, y: clientY };
    if (containerRef.current) containerRef.current.style.cursor = 'grabbing';
  };

  const handleMouseUp = () => {
    draggingRef.current = false;
    if (containerRef.current) containerRef.current.style.cursor = 'grab';
  };

  const onMouseDown = (e: React.MouseEvent) => handleMouseDown(e.clientX, e.clientY);
  const onMouseUp = () => handleMouseUp();
  const onMouseLeave = () => handleMouseUp();

  const onTouchStart = (e: React.TouchEvent) => {
    if (e.touches.length === 0) return;
    handleMouseDown(e.touches[0].clientX, e.touches[0].clientY);
  };
  const onTouchEnd = () => handleMouseUp();

  const onWheel = (e: React.WheelEvent) => {
    if (!e.ctrlKey) return;
    e.preventDefault();
    const zoomFactor = -e.deltaY * 0.001;
    const newScale = Math.min(MAX_SCALE, Math.max(MIN_SCALE, scaleRef.current * (1 + zoomFactor)));
    setScale(newScale);

    // Recalculate grid based on the new scale and viewport size
    const scaledTileSize = TILE_SIZE * newScale;
    const tilesX = Math.ceil(viewport.w / scaledTileSize) + BUFFER_TILES * 2;
    const tilesY = Math.ceil(viewport.h / scaledTileSize) + BUFFER_TILES * 2;

    const offsetX = grid.x - viewport.w / 2;
    const offsetY = grid.y - viewport.h / 2;

    const startGridX = Math.floor(-offsetX / scaledTileSize) - BUFFER_TILES;
    const startGridY = Math.floor(-offsetY / scaledTileSize) - BUFFER_TILES;

    setGrid({ x: grid.x, y: grid.y }); // Update grid based on new scale and viewport size
  };

  // Calculate visible grid bounds
  const scaledTileSize = TILE_SIZE * scale;
  const tilesX = Math.ceil(viewport.w / scaledTileSize) + BUFFER_TILES * 2;
  const tilesY = Math.ceil(viewport.h / scaledTileSize) + BUFFER_TILES * 2;

  const offsetX = grid.x - viewport.w / 2;
  const offsetY = grid.y - viewport.h / 2;

  const startGridX = Math.floor(-offsetX / scaledTileSize) - BUFFER_TILES;
  const startGridY = Math.floor(-offsetY / scaledTileSize) - BUFFER_TILES;

  const dotCoords: { x: number; y: number }[] = [];
  for (let gy = 0; gy < tilesY; gy++) {
    for (let gx = 0; gx < tilesX; gx++) {
      dotCoords.push({
        x: (startGridX + gx) * TILE_SIZE,
        y: (startGridY + gy) * TILE_SIZE,
      });
    }
  }

  return (
    <div
      ref={containerRef}
      className="canvas-container"
      onMouseDown={onMouseDown}
      onMouseUp={onMouseUp}
      onMouseLeave={onMouseLeave}
      onTouchStart={onTouchStart}
      onTouchEnd={onTouchEnd}
      onWheel={onWheel}
      style={{ cursor: 'grab', userSelect: 'none', touchAction: 'none' }}
    >
      <div
        className="tile-layer"
        style={{
          transform: `translate(${grid.x}px, ${grid.y}px) scale(${scale})`,
          transformOrigin: 'center center',
        }}
      >
        {dotCoords.map(({ x, y }, index) => (
          <div
            key={index}
            className="dot"
            style={{
              position: 'absolute',
              left: `${x}px`,
              top: `${y}px`,
              width: '10px',
              height: '10px',
              borderRadius: '50%',
              backgroundColor: 'black',
              transform: 'translate(-50%, -50%)',
            }}
          />
        ))}
      </div>
    </div>
  );
}
