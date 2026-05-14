import type { BacktestRun } from "../../types/backtest";

type Props = {
  runs: BacktestRun[];
};

type Metric = {
  label: string;
  getValue: (run: BacktestRun) => string;
};

const METRICS: Metric[] = [
  {
    label: "Final equity",
    getValue: (run) => run.result.summary.final_equity.toFixed(2),
  },
  {
    label: "PNL",
    getValue: (run) => run.result.summary.pnl.toFixed(2),
  },
  {
    label: "Return",
    getValue: (run) => `${run.result.summary.return_pct.toFixed(2)}%`,
  },
  {
    label: "Orders",
    getValue: (run) => run.result.summary.orders.toLocaleString(),
  },
  {
    label: "Fills",
    getValue: (run) => run.result.summary.fills.toLocaleString(),
  },
  {
    label: "Backtest run",
    getValue: (run) => `${run.result.timing.backtest_run_seconds.toFixed(3)}s`,
  },
  {
    label: "Total",
    getValue: (run) => `${run.result.timing.total_seconds.toFixed(3)}s`,
  },
];

function ComparisonMetricsTable({ runs }: Props) {
  return (
    <div className="comparison-table-wrap">
      <table className="comparison-table">
        <thead>
          <tr>
            <th>Metric</th>
            {runs.map((run) => (
              <th key={run.strategy.id}>{run.strategy.label}</th>
            ))}
          </tr>
        </thead>
        <tbody>
          {METRICS.map((metric) => (
            <tr key={metric.label}>
              <td>{metric.label}</td>
              {runs.map((run) => (
                <td key={run.strategy.id}>{metric.getValue(run)}</td>
              ))}
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
}

export default ComparisonMetricsTable;
