import "./App.css";
import { useState } from "react";
import ApiHealthCard from "./components/layout/ApiHealthCard";
import CompareBacktestsPage from "./pages/CompareBacktestsPage";
import OnlineSimulationPage from "./pages/OnlineSimulationPage";
import SingleBacktestPage from "./pages/SingleBacktestPage";

type Page = "single" | "compare" | "online";

function App() {
  const [page, setPage] = useState<Page>("single");

  return (
    <main className="app">
      <header className="app-header">
        <div>
          <p className="eyebrow">C++ portfolio simulation</p>
          <h1>Trading Engine</h1>
        </div>
      </header>

      <ApiHealthCard />

      <nav className="page-tabs" aria-label="Backtest workflow pages">
        <button
          className={page === "single" ? "page-tab page-tab-active" : "page-tab"}
          type="button"
          onClick={() => setPage("single")}
        >
          Single Run
        </button>
        <button
          className={page === "compare" ? "page-tab page-tab-active" : "page-tab"}
          type="button"
          onClick={() => setPage("compare")}
        >
          Compare
        </button>
        <button
          className={page === "online" ? "page-tab page-tab-active" : "page-tab"}
          type="button"
          onClick={() => setPage("online")}
        >
          Online
        </button>
      </nav>

      {page === "single" && <SingleBacktestPage />}
      {page === "compare" && <CompareBacktestsPage />}
      {page === "online" && <OnlineSimulationPage />}
    </main>
  );
}

export default App;
