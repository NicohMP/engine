import { useState } from "react";
import { runRandomOnlineSimulation } from "../api/backtests";
import AssetPriceChart from "../components/charts/AssetPriceChart";
import EquityCurveChart from "../components/charts/EquityCurveChart";
import TickerSelector from "../components/forms/TickerSelector";
import type {
  OnlineSimulationRequest,
  OnlineSimulationResult,
} from "../types/backtest";

type OnlineFormValues = {
  selectedTickers: string[];
  lookbackMinutes: number;
  interval: string;
  maxPolls: number;
  pollSeconds: number;
  initialCash: number;
  targetNotional: number;
  seed: number;
};

const DEFAULT_VALUES: OnlineFormValues = {
  selectedTickers: ["AAPL", "AMZN"],
  lookbackMinutes: 120,
  interval: "1m",
  maxPolls: 1,
  pollSeconds: 0,
  initialCash: 100000,
  targetNotional: 1000,
  seed: 42,
};

const INTERVALS = ["1m", "2m", "5m", "15m", "30m", "60m", "1d"];

function buildRequest(values: OnlineFormValues): OnlineSimulationRequest {
  return {
    tickers: values.selectedTickers,
    lookback_minutes: values.lookbackMinutes,
    interval: values.interval,
    max_polls: values.maxPolls,
    poll_seconds: values.pollSeconds,
    initial_cash: values.initialCash,
    target_notional: values.targetNotional,
    seed: values.seed,
  };
}

function formatTimestamp(timestamp: number) {
  if (timestamp < 0) {
    return "No rows yet";
  }

  return new Date(timestamp).toLocaleString();
}

function OnlineSimulationPage() {
  const [values, setValues] = useState(DEFAULT_VALUES);
  const [simulation, setSimulation] = useState<OnlineSimulationResult | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [isRunning, setIsRunning] = useState(false);

  function updateValues(nextValues: Partial<OnlineFormValues>) {
    setValues((currentValues) => ({ ...currentValues, ...nextValues }));
  }

  async function handleRun() {
    if (values.selectedTickers.length === 0) {
      setError("Select at least one ticker.");
      return;
    }

    setIsRunning(true);
    setError(null);

    try {
      const result = await runRandomOnlineSimulation(buildRequest(values));
      setSimulation(result);
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
          <p className="section-kicker">Online simulation</p>
          <h2>Random Strategy Live Poll</h2>
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
            <label htmlFor="online-tickers">Tickers</label>
            <TickerSelector
              selectedSymbols={values.selectedTickers}
              onChange={(selectedTickers) => updateValues({ selectedTickers })}
            />
          </div>

          <div className="form-row">
            <label htmlFor="online-lookback">Lookback minutes</label>
            <input
              id="online-lookback"
              min="1"
              type="number"
              value={values.lookbackMinutes}
              onChange={(event) =>
                updateValues({ lookbackMinutes: Number(event.target.value) })
              }
            />
          </div>

          <div className="form-row">
            <label htmlFor="online-interval">Interval</label>
            <select
              id="online-interval"
              value={values.interval}
              onChange={(event) => updateValues({ interval: event.target.value })}
            >
              {INTERVALS.map((interval) => (
                <option key={interval} value={interval}>
                  {interval}
                </option>
              ))}
            </select>
          </div>

          <div className="form-row">
            <label htmlFor="online-max-polls">Polls</label>
            <input
              id="online-max-polls"
              min="1"
              type="number"
              value={values.maxPolls}
              onChange={(event) =>
                updateValues({ maxPolls: Number(event.target.value) })
              }
            />
          </div>

          <div className="form-row">
            <label htmlFor="online-poll-seconds">Seconds between polls</label>
            <input
              id="online-poll-seconds"
              min="0"
              step="0.5"
              type="number"
              value={values.pollSeconds}
              onChange={(event) =>
                updateValues({ pollSeconds: Number(event.target.value) })
              }
            />
          </div>

          <div className="form-row">
            <label htmlFor="online-initial-cash">Initial cash</label>
            <input
              id="online-initial-cash"
              min="1"
              type="number"
              value={values.initialCash}
              onChange={(event) =>
                updateValues({ initialCash: Number(event.target.value) })
              }
            />
          </div>

          <div className="form-row">
            <label htmlFor="online-target-notional">Target notional</label>
            <input
              id="online-target-notional"
              min="1"
              type="number"
              value={values.targetNotional}
              onChange={(event) =>
                updateValues({ targetNotional: Number(event.target.value) })
              }
            />
          </div>

          <div className="form-row">
            <label htmlFor="online-seed">Seed</label>
            <input
              id="online-seed"
              type="number"
              value={values.seed}
              onChange={(event) => updateValues({ seed: Number(event.target.value) })}
            />
          </div>

          <button className="primary-button" type="submit" disabled={isRunning}>
            {isRunning ? "Polling market data..." : "Run Online Poll"}
          </button>

          {error && <p className="error-message">Error: {error}</p>}
        </form>

        <div className="panel summary-panel">
          {simulation ? (
            <>
              <div className="summary-header">
                <div>
                  <p className="section-kicker">Current state</p>
                  <h3>Online Simulation Summary</h3>
                </div>
                <span className="symbol-pill">{simulation.symbols.join(", ")}</span>
              </div>

              <div className="metric-grid">
                <div className="metric">
                  <span className="metric-label">Final equity</span>
                  <strong className="metric-value">
                    {simulation.summary.final_equity.toFixed(2)}
                  </strong>
                </div>
                <div className="metric">
                  <span className="metric-label">Processed slices</span>
                  <strong className="metric-value">
                    {simulation.summary.processed_slices}
                  </strong>
                </div>
                <div className="metric">
                  <span className="metric-label">Orders</span>
                  <strong className="metric-value">{simulation.summary.orders}</strong>
                </div>
                <div className="metric">
                  <span className="metric-label">Fills</span>
                  <strong className="metric-value">{simulation.summary.fills}</strong>
                </div>
                <div className="metric">
                  <span className="metric-label">Polls</span>
                  <strong className="metric-value">{simulation.summary.polls}</strong>
                </div>
                <div className="metric">
                  <span className="metric-label">Total time</span>
                  <strong className="metric-value">
                    {simulation.timing.total_seconds.toFixed(3)}s
                  </strong>
                </div>
              </div>
            </>
          ) : (
            <div className="empty-state">
              <p className="section-kicker">Current state</p>
              <h3>No online poll yet</h3>
              <p>Run a poll to fetch recent bars and step the C++ simulator.</p>
            </div>
          )}
        </div>
      </div>

      {simulation && (
        <>
          <div className="panel online-diagnostics-panel">
            <div className="panel-title-row">
              <div>
                <p className="section-kicker">Polling diagnostics</p>
                <h3>Latest Data Fetches</h3>
              </div>
              <span>{simulation.poll_results.length} polls</span>
            </div>

            <div className="comparison-table-wrap">
              <table className="comparison-table">
                <thead>
                  <tr>
                    <th>Poll</th>
                    <th>Fetched rows</th>
                    <th>New rows</th>
                    <th>New slices</th>
                    <th>Seconds</th>
                  </tr>
                </thead>
                <tbody>
                  {simulation.poll_results.map((poll) => (
                    <tr key={poll.poll}>
                      <td>{poll.poll}</td>
                      <td>{poll.fetched_rows}</td>
                      <td>{poll.new_rows}</td>
                      <td>{poll.new_slices}</td>
                      <td>{poll.seconds.toFixed(3)}</td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>

            <div className="last-seen-list">
              {Object.entries(simulation.last_seen_by_ticker).map(
                ([ticker, timestamp]) => (
                  <span className="status-pill" key={ticker}>
                    {ticker}: {formatTimestamp(timestamp)}
                  </span>
                )
              )}
            </div>
          </div>

          <div className="panel chart-panel">
            <div className="panel-title-row">
              <div>
                <p className="section-kicker">Portfolio equity</p>
                <h3>Online Equity Curve</h3>
              </div>
              <span>{simulation.equity_curve.length} points</span>
            </div>
            <EquityCurveChart points={simulation.equity_curve} />
          </div>

          <div className="panel chart-panel">
            <div className="panel-title-row">
              <div>
                <p className="section-kicker">Asset prices</p>
                <h3>Normalized Close Prices</h3>
              </div>
              <span>Base 100</span>
            </div>
            <AssetPriceChart points={simulation.price_curve} />
          </div>
        </>
      )}
    </section>
  );
}

export default OnlineSimulationPage;
