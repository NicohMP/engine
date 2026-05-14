import type { BacktestResult } from "../types/backtest";

type Props = {
  result: BacktestResult;
};

function BacktestSummary({ result }: Props) {
  return (
    <div>
        {/* Join turns ["AAPL", "AMZN"] into "AAPL, AMZN". */}
        <p>Symbols: {result.symbols.join(", ")}</p>
        {/* toFixed(2) formats numbers with two decimals for display. */}
        <p>Initial equity: {result.summary.initial_equity.toFixed(2)}</p>
        <p>Final equity: {result.summary.final_equity.toFixed(2)}</p>
        <p>PNL: {result.summary.pnl.toFixed(2)}</p>
        <p>Return: {result.summary.return_pct.toFixed(2)}%</p>
        <p>Orders: {result.summary.orders}</p>
        <p>Fills: {result.summary.fills}</p>
        <p>Equity points: {result.summary.equity_points}</p>
    </div>
  );
}

export default BacktestSummary;
