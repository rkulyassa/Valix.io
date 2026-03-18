import { useCallback, useEffect, useRef, useState } from "react";
import { PixiGrid } from "./PixiGrid";
import { TileStatuses } from "./types";
import type { Player, Tile } from "./types";

function App() {
  const wsRef = useRef<WebSocket | null>(null);
  const [gridSize, setGridSize] = useState<number>(0);
  const [pid, setPid] = useState<number>(-1);
  const [grid, setGrid] = useState<Tile[][]>([]);
  const [players, setPlayers] = useState<Player[]>([]);

  const handleWSMessage = useCallback(
    (e: MessageEvent<ArrayBuffer>) => {
      const data = new DataView(e.data);
      let offset = 0;
      const opcode = data.getUint8(offset++);
      switch (opcode) {
        case 0:
          setPid(data.getUint8(offset++));
          setGridSize(data.getUint8(offset++));
          // console.log(pid);
          break;
        case 1: {
          const newGrid: Tile[][] = [];
          for (let r = 0; r < gridSize; r++) {
            const row: Tile[] = [];
            for (let c = 0; c < gridSize; c++) {
              const ownerPid = data.getUint8(offset++);
              const status = TileStatuses[data.getUint8(offset++)];
              row.push({ ownerPid, status });
            }
            newGrid.push(row);
          }
          setGrid(newGrid);

          const newPlayers: Player[] = [];
          const playerCount = data.getUint8(offset++);
          for (let i = 0; i < playerCount; i++) {
            newPlayers.push({
              pid: data.getUint8(offset++),
              row: data.getUint8(offset++),
              col: data.getUint8(offset++),
            });
          }
          setPlayers(newPlayers);
          break;
        }
      }
    },
    [gridSize],
  );

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

  return (
    <PixiGrid
      grid={grid}
      players={players}
      pid={pid}
      onKeyDown={handleKeyDown}
    />
  );
}

export default App;
