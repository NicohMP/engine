import { useState } from "react";
import { runBuyHoldBacktest } from "../api/backtests";
import type { BacktestResult } from "../types/backtest";
import BacktestSummary from "./BacktestSummary";
import EquityCurveChart from "./EquityCurveChart";

function BuyHoldBacktestPanel() {
  const [backtest, setBacktest] = useState<BacktestResult | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [isRunning, setIsRunning] = useState(false);

  const [tickersText, setTickersText] = useState("AAPL,AMZN");
  const [start, setStart] = useState("2026-01-01");
  const [end, setEnd] = useState("2026-05-01");
  const [initialCash, setInitialCash] = useState(100000);

  async function handleRun() {
    const tickers = tickersText
      .split(",")
      .map((ticker) => ticker.trim().toUpperCase())
      .filter(Boolean);

    if (tickers.length === 0) {
      setError("Enter at least one ticker.");
      return;
    }

    if (initialCash <= 0) {
      setError("Initial cash must be positive.");
      return;
    }

    setIsRunning(true);
    setError(null);
    setBacktest(null);

    try {
      const result = await runBuyHoldBacktest({
        tickers,
        start,
        end,
        initial_cash: initialCash,
      });

      setBacktest(result);
    } catch (err) {
      setError(err instanceof Error ? err.message : "Unknown error");
    } finally {
      setIsRunning(false);
    }
  }

  return (
    <section className="backtest-section">
      <div className="section-heading">
        <div>
          <p className="section-kicker">Strategy</p>
          <h2>Buy and Hold Backtest</h2>
        </div>
      </div>

      <div className="backtest-grid">
        <form
          className="panel backtest-form"
          onSubmit={(event) => {
            event.preventDefault();
            void handleRun();
          }}
        >
          <div className="form-row form-row-wide">
            <label htmlFor="buy-hold-tickers">Tickers</label>
            <input
              id="buy-hold-tickers"
              value={tickersText}
              onChange={(event) => setTickersText(event.target.value)}
              placeholder="AAPL,AMZN"
            />
          </div>

          <div className="form-row">
            <label htmlFor="buy-hold-start-date">Start date</label>
            <input
              id="buy-hold-start-date"
              type="date"
              value={start}
              onChange={(event) => setStart(event.target.value)}
            />
          </div>

          <div className="form-row">
            <label htmlFor="buy-hold-end-date">End date</label>
            <input
              id="buy-hold-end-date"
              type="date"
              value={end}
              onChange={(event) => setEnd(event.target.value)}
            />
          </div>

          <div className="form-row form-row-wide">
            <label htmlFor="buy-hold-initial-cash">Initial cash</label>
            <input
              id="buy-hold-initial-cash"
              type="number"
              min="1"
              value={initialCash}
              onChange={(event) => setInitialCash(Number(event.target.value))}
            />
          </div>

          <button className="primary-button" type="submit" disabled={isRunning}>
            {isRunning ? "Running backtest..." : "Run Buy and Hold Backtest"}
          </button>

          {error && <p className="error-message">Error: {error}</p>}
        </form>

        <div className="panel summary-panel">
          {backtest ? (
            <BacktestSummary result={backtest} />
          ) : (
            <div className="empty-state">
              <p className="section-kicker">Results</p>
              <h3>No run yet</h3>
              <p>Run a buy and hold backtest to see portfolio metrics here.</p>
            </div>
          )}
        </div>
      </div>

      {backtest && (
        <div className="panel chart-panel">
          <div className="panel-title-row">
            <div>
              <p className="section-kicker">Portfolio equity</p>
              <h3>Equity Curve</h3>
            </div>
            <span>{backtest.equity_curve.length} points</span>
          </div>
          <EquityCurveChart points={backtest.equity_curve} />
        </div>
      )}
    </section>
  );
}

export default BuyHoldBacktestPanel;
