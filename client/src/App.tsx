import { useEffect, useRef } from "react";
import { Application, /*Container,*/ Graphics } from "pixi.js";
import { initDevtools } from "@pixi/devtools";

function App() {
  const containerRef = useRef<HTMLDivElement | null>(null);
  const appRef = useRef<Application | null>(null);
  const wsRef = useRef<WebSocket | null>(null);

  const initPixi = async () => {
    const app = new Application();
    initDevtools({ app });

    await app.init({
      backgroundColor: 0x1e1e2e,
      resizeTo: containerRef.current || undefined,
    });
    appRef.current = app;

    if (containerRef.current) {
      containerRef.current.innerHTML = ""; // fix race condition
      containerRef.current.appendChild(app.canvas);
    }

    const GRID_ROWS = 10;
    const GRID_COLS = 10;
    const TILE_SIZE = 40; // px

    // Optional: Clear the stage first
    // app.stage.removeChildren();

    const grid = new Graphics();
    // grid.lineStyle({ width: 2, color: 0x6c7086, alpha: 0.8 });
    grid.setStrokeStyle({
      width: 2,
      color: 0x6c7086,
    });
    grid.setFillStyle({
      color: 0x313244,
    });

    const offset = 100;
    for (let r = 0; r < GRID_ROWS; r++) {
      for (let c = 0; c < GRID_COLS; c++) {
        grid.rect(
          offset + c * TILE_SIZE,
          offset + r * TILE_SIZE,
          TILE_SIZE,
          TILE_SIZE,
        );
      }
    }

    // // Draw vertical lines
    // for (let c = 0; c <= GRID_COLS; c++) {
    //   grid.moveTo(c * TILE_SIZE, 0);
    //   grid.lineTo(c * TILE_SIZE, GRID_ROWS * TILE_SIZE);
    // }

    // // Draw horizontal lines
    // for (let r = 0; r <= GRID_ROWS; r++) {
    //   grid.moveTo(0, r * TILE_SIZE);
    //   grid.lineTo(GRID_COLS * TILE_SIZE, r * TILE_SIZE);
    // }

    grid.fill();
    grid.stroke();

    app.stage.addChild(grid);
  };

  const handleWSMessage = (e: MessageEvent<string>) => {
    const message = e.data;
    const tiles = message.split(",");
    console.log(tiles);
  };

  useEffect(() => {
    const ws = new WebSocket("ws://localhost:9001");
    wsRef.current = ws;

    if (ws) {
      ws.addEventListener("message", handleWSMessage);
    } else {
      // cannot connect to ws
    }

    initPixi();

    // auto-focus the container for keydown events
    const container = containerRef.current;
    if (!container) return;
    container.focus();

    return () => {
      if (appRef.current) {
        appRef.current.destroy(true, true);
        appRef.current = null;
      }
    };
  }, []);

  const handleKeyDown = (event: React.KeyboardEvent<HTMLDivElement>) => {
    console.log(event.key);
    const ws = wsRef.current;
    if (!ws) return;

    ws.send(event.key);
  };

  return (
    <>
      <div
        tabIndex={0} // make focusable for use with keydown events
        className="h-screen"
        ref={containerRef}
        onKeyDown={handleKeyDown}
      />
    </>
  );
}

export default App;
