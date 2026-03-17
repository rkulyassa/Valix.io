import { initDevtools } from "@pixi/devtools";
import { Application, Graphics } from "pixi.js";
import { useEffect, useRef } from "react";

export interface Tile {
  owner: number;
}

interface PixiGridProps {
  grid: Tile[][];
  onKeyDown?: (event: React.KeyboardEvent<HTMLDivElement>) => void;
}

export function PixiGrid({ grid, onKeyDown }: PixiGridProps) {
  const containerRef = useRef<HTMLDivElement | null>(null);
  const appRef = useRef<Application | null>(null);
  const gridGraphicsRef = useRef<Graphics | null>(null);

  useEffect(() => {
    const setupPixi = async () => {
      const app = new Application();
      initDevtools({ app });

      await app.init({
        backgroundColor: 0x1e1e2e,
        resizeTo: containerRef.current || undefined,
      });
      appRef.current = app;

      if (containerRef.current) {
        containerRef.current.innerHTML = "";
        containerRef.current.appendChild(app.canvas);
        containerRef.current.focus();
      }

      const gridGraphics = new Graphics();
      gridGraphicsRef.current = gridGraphics;
      app.stage.addChild(gridGraphics);
    };

    setupPixi();

    return () => {
      if (appRef.current) {
        appRef.current.destroy(true, true);
        appRef.current = null;
      }
    };
  }, []);

  useEffect(() => {
    const graphics = gridGraphicsRef.current;
    if (!graphics) return;
    graphics.clear();
    const TILE_SIZE_PX = 20;
    const offset = 100;
    for (let r = 0; r < grid.length; r++) {
      for (let c = 0; c < grid[0].length; c++) {
        const fillColor = grid[r][c].owner === 0 ? 0x313244 : 0xa6e3a1;
        graphics
          .rect(
            offset + c * TILE_SIZE_PX,
            offset + r * TILE_SIZE_PX,
            TILE_SIZE_PX,
            TILE_SIZE_PX,
          )
          .fill(fillColor)
          .stroke({
            width: 2,
            color: 0x6c7086,
          });
      }
    }
  }, [grid]);

  return (
    <div
      ref={containerRef}
      className="h-screen"
      onKeyDown={onKeyDown}
      tabIndex={0} // make focusable for use with keydown events
    />
  );
}
