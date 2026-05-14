import { useState } from "react";
import {
  runBacktest,
  STRATEGIES,
} from "../api/backtests";
import AssetPriceChart from "../components/charts/AssetPriceChart";
import EquityCurveChart from "../components/charts/EquityCurveChart";
import BacktestForm from "../components/forms/BacktestForm";
import BacktestSummary from "../components/results/BacktestSummary";
import type {
  BacktestFormValues,
  BacktestResult,
  StrategyId,
} from "../types/backtest";

const DEFAULT_FORM_VALUES: BacktestFormValues = {
  selectedTickers: ["AAPL", "AMZN"],
  start: "2026-01-01",
  end: "2026-05-01",
  initialCash: 100000,
  targetNotional: 1000,
  seed: 42,
};

function SingleBacktestPage() {
  const [strategy, setStrategy] = useState<StrategyId>("random");
  const [values, setValues] = useState(DEFAULT_FORM_VALUES);
  const [backtest, setBacktest] = useState<BacktestResult | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [isRunning, setIsRunning] = useState(false);

  const selectedStrategy = STRATEGIES.find((option) => option.id === strategy);
  const strategyLabel = selectedStrategy?.label ?? "Strategy";

  async function handleRun() {
    if (values.selectedTickers.length === 0) {
      setError("Select at least one ticker.");
      return;
    }

    if (values.initialCash <= 0) {
      setError("Initial cash must be positive.");
      return;
    }

    if (strategy === "random" && values.targetNotional <= 0) {
      setError("Target notional must be positive.");
      return;
    }

    setIsRunning(true);
    setError(null);
    setBacktest(null);

    try {
      const result = await runBacktest(strategy, values);
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
          <p className="section-kicker">Single run</p>
          <h2>{strategyLabel} Backtest</h2>
        </div>

        <div className="strategy-switch" aria-label="Strategy selector">
          {STRATEGIES.map((option) => (
            <button
              className={
                strategy === option.id
                  ? "strategy-button strategy-button-active"
                  : "strategy-button"
              }
              key={option.id}
              type="button"
              onClick={() => {
                setStrategy(option.id);
                setBacktest(null);
                setError(null);
              }}
            >
              {option.label}
            </button>
          ))}
        </div>
      </div>

      <div className="backtest-grid">
        <BacktestForm
          strategy={strategy}
          values={values}
          isRunning={isRunning}
          submitLabel={`Run ${strategyLabel} Backtest`}
          runningLabel="Running backtest..."
          error={error}
          onChange={setValues}
          onSubmit={() => void handleRun()}
        />

        <div className="panel summary-panel">
          {backtest ? (
            <BacktestSummary result={backtest} />
          ) : (
            <div className="empty-state">
              <p className="section-kicker">Results</p>
              <h3>No run yet</h3>
              <p>Run a strategy backtest to see portfolio metrics here.</p>
            </div>
          )}
        </div>
      </div>

      {backtest && (
        <>
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

          <div className="panel chart-panel">
            <div className="panel-title-row">
              <div>
                <p className="section-kicker">Asset prices</p>
                <h3>Normalized Close Prices</h3>
              </div>
              <span>Base 100</span>
            </div>
            <AssetPriceChart points={backtest.price_curve} />
          </div>
        </>
      )}
    </section>
  );
}

export default SingleBacktestPage;
