import "./App.css";
import ApiHealthCard from "./components/ApiHealthCard";
import RandomBacktestPanel from "./components/RandomBacktestPanel";

function App() {
  return (
    <main className="app">
      <header className="app-header">
        <div>
          <p className="eyebrow">C++ portfolio simulation</p>
          <h1>Trading Engine</h1>
        </div>
      </header>

      <ApiHealthCard />

      <RandomBacktestPanel />
    </main>
  );
}

export default App;
