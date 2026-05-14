import "./App.css";
import { useState } from "react";
import ApiHealthCard from "./components/ApiHealthCard";
import BuyHoldBacktestPanel from "./components/BuyHoldBacktestPanel";
import RandomBacktestPanel from "./components/RandomBacktestPanel";

type Page = "random" | "buy-hold";

function App() {
  const [page, setPage] = useState<Page>("random");

  return (
    <main className="app">
      <header className="app-header">
        <div>
          <p className="eyebrow">C++ portfolio simulation</p>
          <h1>Trading Engine</h1>
        </div>
      </header>

      <ApiHealthCard />

      <nav className="page-tabs" aria-label="Backtest strategy pages">
        <button
          className={page === "random" ? "page-tab page-tab-active" : "page-tab"}
          type="button"
          onClick={() => setPage("random")}
        >
          Random
        </button>
        <button
          className={
            page === "buy-hold" ? "page-tab page-tab-active" : "page-tab"
          }
          type="button"
          onClick={() => setPage("buy-hold")}
        >
          Buy and Hold
        </button>
      </nav>

      {page === "random" ? <RandomBacktestPanel /> : <BuyHoldBacktestPanel />}
    </main>
  );
}

export default App;
