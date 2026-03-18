import { useCallback, useEffect, useRef, useState } from "react";
import { PixiGrid } from "./PixiGrid";
import { type Tile, TileStatuses } from "./types";

export const WORLD_GRID_SIZE = 25;

function App() {
  const wsRef = useRef<WebSocket | null>(null);
  const [grid, setGrid] = useState<Tile[][]>(
    Array.from({ length: WORLD_GRID_SIZE }, () =>
      Array.from({ length: WORLD_GRID_SIZE }, () => ({
        ownerPid: 0,
        status: "Empty",
      })),
    ),
  );

  const handleWSMessage = useCallback((e: MessageEvent<ArrayBuffer>) => {
    const data = new DataView(e.data);
    const nextGrid: Tile[][] = [];

    let offset = 0;
    for (let r = 0; r < WORLD_GRID_SIZE; r++) {
      const row: Tile[] = [];
      for (let c = 0; c < WORLD_GRID_SIZE; c++) {
        const ownerPid = data.getUint8(offset++);
        const status = TileStatuses[data.getUint8(offset++)];
        row.push({ ownerPid, status });
      }
      nextGrid.push(row);
    }

    setGrid(nextGrid);
  }, []);

  useEffect(() => {
    const ws = new WebSocket("ws://localhost:9001");
    wsRef.current = ws;

    ws.binaryType = "arraybuffer";
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
