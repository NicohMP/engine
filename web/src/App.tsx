import "./App.css";
import ApiHealthCard from "./components/ApiHealthCard";
import RandomBacktestPanel from "./components/RandomBacktestPanel";

function App() {
  return (
    <main>
      <h1>Trading Engine</h1>

      <ApiHealthCard />

      <RandomBacktestPanel />
    </main>
  );
}

export default App;
