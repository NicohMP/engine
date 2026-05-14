import type { BacktestResult } from "../types/backtest";

type Props = {
  result: BacktestResult;
};

function BacktestSummary({ result }: Props) {
  const metrics = [
    ["Initial equity", result.summary.initial_equity.toFixed(2)],
    ["Final equity", result.summary.final_equity.toFixed(2)],
    ["PNL", result.summary.pnl.toFixed(2)],
    ["Return", `${result.summary.return_pct.toFixed(2)}%`],
    ["Orders", result.summary.orders.toLocaleString()],
    ["Fills", result.summary.fills.toLocaleString()],
    ["Equity points", result.summary.equity_points.toLocaleString()],
    ["Data fetch", `${result.timing.data_fetch_seconds.toFixed(3)}s`],
    ["Backtest run", `${result.timing.backtest_run_seconds.toFixed(3)}s`],
    ["Total", `${result.timing.total_seconds.toFixed(3)}s`],
  ];

  return (
    <div>
      <div className="summary-header">
        <div>
          <p className="section-kicker">Results</p>
          <h3>Backtest Summary</h3>
        </div>
        <span className="symbol-pill">{result.symbols.join(", ")}</span>
      </div>

      <div className="metric-grid">
        {metrics.map(([label, value]) => (
          <div className="metric" key={label}>
            <span className="metric-label">{label}</span>
            <strong className="metric-value">{value}</strong>
          </div>
        ))}
      </div>
    </div>
  );
}

export default BacktestSummary;
