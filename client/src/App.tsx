import { useCallback, useEffect, useRef, useState } from "react";
import { PixiGrid, type Tile } from "./PixiGrid";

function App() {
  // const containerRef = useRef<HTMLDivElement | null>(null);
  // const appRef = useRef<Application | null>(null);
  const wsRef = useRef<WebSocket | null>(null);
  const [grid, setGrid] = useState<Tile[][]>(
    Array.from({ length: 10 }, () =>
      Array.from({ length: 10 }, () => ({ owner: 0 })),
    ),
  );
  // const gridGraphicsRef = useRef<Graphics | null>(null);

  // const drawGrid = () => {
  //   if (!gridGraphicsRef.current) return;
  //   const gridGraphics = gridGraphicsRef.current;
  //   gridGraphics.clear();

  //   const TILE_SIZE = 40;

  //   console.log(grid);
  //   const offset = 100;
  //   for (let r = 0; r < grid.length; r++) {
  //     for (let c = 0; c < grid[0].length; c++) {
  //       const fillColor = grid[r][c].owner === 0 ? 0x313244 : 0xff0000;
  //       gridGraphics
  //         .rect(
  //           offset + c * TILE_SIZE,
  //           offset + r * TILE_SIZE,
  //           TILE_SIZE,
  //           TILE_SIZE,
  //         )
  //         .fill(fillColor)
  //         .stroke({
  //           width: 2,
  //           color: 0x6c7086,
  //         });
  //     }
  //   }

  //   // gridGraphics.fill();
  //   // gridGraphics.stroke();
  // };

  // const initPixi = async () => {
  //   const app = new Application();
  //   initDevtools({ app });

  //   await app.init({
  //     backgroundColor: 0x1e1e2e,
  //     resizeTo: containerRef.current || undefined,
  //   });
  //   appRef.current = app;

  //   if (containerRef.current) {
  //     containerRef.current.innerHTML = ""; // fix race condition
  //     containerRef.current.appendChild(app.canvas);
  //   }

  //   const gridGraphics = new Graphics();
  //   gridGraphicsRef.current = gridGraphics;

  //   app.stage.addChild(gridGraphics);
  // };

  const handleWSMessage = useCallback((e: MessageEvent<string>) => {
    const message = e.data;
    const tileOwners = message.split(",").map(Number);
    // console.log(tileOwners);
    // const newGrid = grid.map((row) => row.slice());
    const nextGrid: Tile[][] = [];
    const numRows = 10;
    const numCols = 10;

    for (let r = 0; r < numRows; r++) {
      const row: Tile[] = [];
      for (let c = 0; c < numCols; c++) {
        const index = r * numCols + c;
        row.push({ owner: tileOwners[index] });
      }
      nextGrid.push(row);
    }
    // for (let i = 0; i < tileOwners.length; i++) {
    //   const row = Math.floor(i / numCols);
    //   const col = i % numCols;
    //   newGrid[row][col] = { owner: tileOwners[i] };
    // }
    // console.log("newgrid", newGrid);

    // console.log(newGrid);
    setGrid(nextGrid);
    // drawGrid();
  }, []);

  useEffect(() => {
    const ws = new WebSocket("ws://localhost:9001");
    wsRef.current = ws;

    // if (ws) {
    ws.addEventListener("message", handleWSMessage);
    // } else {
    // cannot connect to ws
    // }

    // initPixi();

    // auto-focus the container for keydown events
    // const container = containerRef.current;
    // if (!container) return;
    // container.focus();

    return () => {
      // if (appRef.current) {
      //   appRef.current.destroy(true, true);
      //   appRef.current = null;
      // }
      ws.close();
      wsRef.current = null;
    };
  }, [handleWSMessage]);

  // useEffect(() => {
  //   if (!gridGraphicsRef.current) return;
  //   drawGrid();
  // }, [grid]);

  const handleKeyDown = (event: React.KeyboardEvent<HTMLDivElement>) => {
    console.log(event.key);
    const ws = wsRef.current;
    if (!ws) return;

    ws.send(event.key);
  };

  return <PixiGrid grid={grid} onKeyDown={handleKeyDown} />;
}

export default App;
