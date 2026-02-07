const host = window.location.href.startsWith("http://localhost")
  ? "localhost"
  : "134.209.161.181";
const WS_URL = `ws://${host}:3000`;

function App() {}

export default App;
