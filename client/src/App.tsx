import { useEffect, useRef } from "react";
import { Application, Container, Graphics } from "pixi.js";

function App() {
  const containerRef = useRef<HTMLDivElement | null>(null);
  const appRef = useRef<Application | null>(null);
  const mouseCircleRef = useRef<Graphics | null>(null);

  useEffect(() => {
    const initPixi = async () => {
      const app = new Application();

      await app.init({
        backgroundColor: 0x1e1e2e,
        resizeTo: containerRef.current || undefined,
      });

      if (containerRef.current) {
        containerRef.current.innerHTML = ""; // fix race condition
        containerRef.current.appendChild(app.canvas);
      }

      appRef.current = app;

      const circle = new Graphics().circle(0, 0, 50).fill(0xcba6f7);
      if (appRef.current) {
        appRef.current.stage.addChild(circle);
        mouseCircleRef.current = circle;
      }
    };

    initPixi();

    return () => {
      if (appRef.current) {
        appRef.current.destroy(true, true);
        appRef.current = null;
      }
    };
  }, []);

  const handleMouseMove = (event: React.MouseEvent<HTMLDivElement>) => {
    const app = appRef.current;
    if (!app || !mouseCircleRef.current) return;

    mouseCircleRef.current.x = event.clientX;
    mouseCircleRef.current.y = event.clientY;
  };

  return (
    <>
      <div
        className="h-screen"
        ref={containerRef}
        onMouseMove={handleMouseMove}
      />
    </>
  );
}

export default App;
