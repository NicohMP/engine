import type { EquityPoint } from "../types/backtest";

import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  Tooltip,
  ResponsiveContainer,
  CartesianGrid,
} from "recharts";

type Props = {
  points: EquityPoint[];
};

function EquityCurveChart({ points }: Props) {
  const chartData = points.map((point) => ({
    date: new Date(point.timestamp).toLocaleDateString(),
    equity: point.equity,
  }));

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

          <Line
            dataKey="equity"
            dot={false}
            stroke="#2563eb"
            strokeWidth={2}
            type="monotone"
          />
        </LineChart>
      </ResponsiveContainer>
    </div>
  );
}

export default EquityCurveChart;
