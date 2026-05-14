import { useState } from "react";
import {
  runBacktest,
  STRATEGIES,
} from "../api/backtests";
import AssetPriceChart from "../components/charts/AssetPriceChart";
import ComparisonEquityChart from "../components/charts/ComparisonEquityChart";
import BacktestForm from "../components/forms/BacktestForm";
import ComparisonMetricsTable from "../components/results/ComparisonMetricsTable";
import type { BacktestFormValues, BacktestRun } from "../types/backtest";

const DEFAULT_FORM_VALUES: BacktestFormValues = {
  selectedTickers: ["AAPL", "AMZN"],
  start: "2026-01-01",
  end: "2026-05-01",
  initialCash: 100000,
  targetNotional: 1000,
  seed: 42,
};

function CompareBacktestsPage() {
  const [values, setValues] = useState(DEFAULT_FORM_VALUES);
  const [runs, setRuns] = useState<BacktestRun[]>([]);
  const [error, setError] = useState<string | null>(null);
  const [isRunning, setIsRunning] = useState(false);

  async function handleRun() {
    if (values.selectedTickers.length === 0) {
      setError("Select at least one ticker.");
      return;
    }

    if (values.initialCash <= 0 || values.targetNotional <= 0) {
      setError("Initial cash and target notional must be positive.");
      return;
    }

    setIsRunning(true);
    setError(null);
    setRuns([]);

    try {
      const [randomResult, buyHoldResult] = await Promise.all([
        runBacktest("random", values),
        runBacktest("buy_hold", values),
      ]);

      setRuns([
        { strategy: STRATEGIES[0], result: randomResult },
        { strategy: STRATEGIES[1], result: buyHoldResult },
      ]);
    } catch (err) {
      setError(err instanceof Error ? err.message : "Unknown error");
    } finally {
      setIsRunning(false);
    }
  }

  const assetPricePoints = runs[0]?.result.price_curve ?? [];

  return (
    <section className="backtest-section">
      <div className="section-heading">
        <div>
          <p className="section-kicker">Comparison</p>
          <h2>Random vs Buy and Hold</h2>
        </div>
      </div>

      <div className="backtest-grid comparison-grid">
        <BacktestForm
          strategy="random"
          values={values}
          isRunning={isRunning}
          submitLabel="Run Comparison"
          runningLabel="Running comparison..."
          error={error}
          onChange={setValues}
          onSubmit={() => void handleRun()}
        />

        <div className="panel summary-panel">
          {runs.length > 0 ? (
            <>
              <div className="summary-header">
                <div>
                  <p className="section-kicker">Results</p>
                  <h3>Strategy Comparison</h3>
                </div>
                <span className="symbol-pill">{runs[0].result.symbols.join(", ")}</span>
              </div>
              <ComparisonMetricsTable runs={runs} />
            </>
          ) : (
            <div className="empty-state">
              <p className="section-kicker">Results</p>
              <h3>No comparison yet</h3>
              <p>Run both strategies to compare portfolio metrics here.</p>
            </div>
          )}
        </div>
      </div>

      {runs.length > 0 && (
        <>
          <div className="panel chart-panel">
            <div className="panel-title-row">
              <div>
                <p className="section-kicker">Portfolio equity</p>
                <h3>Strategy Equity Comparison</h3>
              </div>
              <span>{runs.length} strategies</span>
            </div>
            <ComparisonEquityChart runs={runs} />
          </div>

          <div className="panel chart-panel">
            <div className="panel-title-row">
              <div>
                <p className="section-kicker">Asset prices</p>
                <h3>Normalized Close Prices</h3>
              </div>
              <span>Base 100</span>
            </div>
            <AssetPriceChart points={assetPricePoints} />
          </div>
        </>
      )}
    </section>
  );
}

export default CompareBacktestsPage;
