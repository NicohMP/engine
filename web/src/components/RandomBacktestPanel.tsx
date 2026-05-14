import { useState } from "react";
import { runRandomBacktest } from "../api/backtests";
import type { BacktestResult } from "../types/backtest";
import BacktestSummary from "./BacktestSummary";
import EquityCurveChart from "./EquityCurveChart";


function RandomBacktestPanel() {
  const [backtest, setBacktest] = useState<BacktestResult | null>(null);

  async function handleRun() {
    const result = await runRandomBacktest({
      tickers: ["AAPL", "AMZN"],
      start: "2026-01-01",
      end: "2026-05-01",
      target_notional: 1000,
      initial_cash: 100000,
      seed: 42,
    });

    setBacktest(result);
  }

  return (
    <section>
      <button onClick={handleRun}>Run</button>
      {backtest && (
        <>
          <BacktestSummary result={backtest}/>
          <EquityCurveChart points={backtest.equity_curve} />
        </>
      )}
    </section>
  );
}

export default RandomBacktestPanel;
