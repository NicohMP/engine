import type { PricePoint } from "../types/backtest";

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
  points: PricePoint[];
};

const COLORS = ["#2563eb", "#16a34a", "#dc2626", "#9333ea", "#ea580c"];

function AssetPriceChart({ points }: Props) {
  const firstCloseByTicker = new Map<string, number>();
  const rowsByTimestamp = new Map<number, Record<string, string | number>>();

  for (const point of points) {
    if (!firstCloseByTicker.has(point.ticker)) {
      firstCloseByTicker.set(point.ticker, point.close);
    }

    const firstClose = firstCloseByTicker.get(point.ticker);

    if (!firstClose || firstClose <= 0) {
      continue;
    }

    const normalizedClose = (point.close / firstClose) * 100;

    const row =
      rowsByTimestamp.get(point.timestamp) ?? {
        date: new Date(point.timestamp).toLocaleDateString(),
      };

    row[point.ticker] = normalizedClose;
    rowsByTimestamp.set(point.timestamp, row);
  }

  const tickers = Array.from(firstCloseByTicker.keys());
  const chartData = Array.from(rowsByTimestamp.entries())
    .sort(([a], [b]) => a - b)
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

          {tickers.map((ticker, index) => (
            <Line
              key={ticker}
              dataKey={ticker}
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

export default AssetPriceChart;
