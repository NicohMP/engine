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
    <div style={{ width: "100%", height: 320 }}>
      <ResponsiveContainer>
        <LineChart data={chartData}>
          <CartesianGrid strokeDasharray="3 3" />

          <XAxis dataKey="date" />

          <YAxis
            domain={["dataMin", "dataMax"]}
            tickFormatter={(value) => value.toFixed(0)}
          />

          <Tooltip
            formatter={(value) =>
              typeof value === "number" ? value.toFixed(2) : value
            }
          />

          <Line
            type="monotone"
            dataKey="equity"
            stroke="#2563eb"
            strokeWidth={2}
            dot={false}
          />
        </LineChart>
      </ResponsiveContainer>
    </div>
  );
}

export default EquityCurveChart;
