import { useCallback, useEffect, useRef, useState } from "react";
import { PixiGrid, type Tile } from "./PixiGrid";

export const WORLD_GRID_SIZE = 25;

function App() {
  const wsRef = useRef<WebSocket | null>(null);
  const [grid, setGrid] = useState<Tile[][]>(
    Array.from({ length: WORLD_GRID_SIZE }, () =>
      Array.from({ length: WORLD_GRID_SIZE }, () => ({ owner: 0 })),
    ),
  );

  const handleWSMessage = useCallback((e: MessageEvent<string>) => {
    const message = e.data;
    const tileOwners = message.split(",").map(Number);
    const nextGrid: Tile[][] = [];

    for (let r = 0; r < WORLD_GRID_SIZE; r++) {
      const row: Tile[] = [];
      for (let c = 0; c < WORLD_GRID_SIZE; c++) {
        const index = r * WORLD_GRID_SIZE + c;
        row.push({ owner: tileOwners[index] });
      }
      nextGrid.push(row);
    }

    setGrid(nextGrid);
  }, []);

  useEffect(() => {
    const ws = new WebSocket("ws://localhost:9001");
    wsRef.current = ws;

    ws.addEventListener("message", handleWSMessage);

    return () => {
      ws.close();
      wsRef.current = null;
    };
  }, [handleWSMessage]);

  const handleKeyDown = (event: React.KeyboardEvent<HTMLDivElement>) => {
    console.log(event.key);
    const ws = wsRef.current;
    if (!ws) return;

    ws.send(event.key);
  };

  return <PixiGrid grid={grid} onKeyDown={handleKeyDown} />;
}

export default App;
