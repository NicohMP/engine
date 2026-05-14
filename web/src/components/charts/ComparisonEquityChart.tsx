import type { BacktestRun } from "../../types/backtest";

import {
  CartesianGrid,
  Line,
  LineChart,
  ResponsiveContainer,
  Tooltip,
  XAxis,
  YAxis,
} from "recharts";

type Props = {
  runs: BacktestRun[];
};

const COLORS = ["#2563eb", "#16a34a", "#dc2626", "#9333ea"];

function ComparisonEquityChart({ runs }: Props) {
  const rowsByTimestamp = new Map<number, Record<string, string | number>>();

  for (const run of runs) {
    for (const point of run.result.equity_curve) {
      const row =
        rowsByTimestamp.get(point.timestamp) ??
        {
          date: new Date(point.timestamp).toLocaleDateString(),
        };

      row[run.strategy.label] = point.equity;
      rowsByTimestamp.set(point.timestamp, row);
    }
  }

  const chartData = Array.from(rowsByTimestamp.entries())
    .sort(([leftTimestamp], [rightTimestamp]) => leftTimestamp - rightTimestamp)
    .map(([, row]) => row);

  return (
    <div className="equity-chart">
      <ResponsiveContainer>
        <LineChart data={chartData}>
          <CartesianGrid stroke="#e4e7ec" strokeDasharray="3 3" />

          <XAxis
            dataKey="date"
            minTickGap={32}
            tick={{ fill: "#667085", fontSize: 12 }}
            tickLine={false}
          />

          <YAxis
            domain={["dataMin", "dataMax"]}
            tick={{ fill: "#667085", fontSize: 12 }}
            tickFormatter={(value) => value.toFixed(0)}
            tickLine={false}
            width={72}
          />

          <Tooltip
            contentStyle={{
              border: "1px solid #d0d5dd",
              borderRadius: 8,
              boxShadow: "0 8px 24px rgba(16, 24, 40, 0.12)",
            }}
            formatter={(value) =>
              typeof value === "number" ? value.toFixed(2) : value
            }
          />

          {runs.map((run, index) => (
            <Line
              key={run.strategy.id}
              dataKey={run.strategy.label}
              dot={false}
              stroke={COLORS[index % COLORS.length]}
              strokeWidth={2}
              type="monotone"
            />
          ))}
        </LineChart>
      </ResponsiveContainer>
    </div>
  );
}

export default ComparisonEquityChart;
