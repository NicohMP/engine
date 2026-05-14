import { apiFetch } from "./client";
import type {
  BacktestResult,
  BacktestFormValues,
  BuyHoldBacktestRequest,
  RandomBacktestRequest,
  StrategyId,
  StrategyOption,
} from "../types/backtest";

export const STRATEGIES: StrategyOption[] = [
  { id: "random", label: "Random" },
  { id: "buy_hold", label: "Buy and Hold" },
];

export async function runRandomBacktest(request: RandomBacktestRequest) {
  const response = await apiFetch("/backtests/random", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(request),
  });

  return response.json() as Promise<BacktestResult>;
}

export function buildBacktestRequest(
  strategy: StrategyId,
  values: BacktestFormValues
): RandomBacktestRequest | BuyHoldBacktestRequest {
  const baseRequest = {
    tickers: values.selectedTickers,
    start: values.start,
    end: values.end,
    initial_cash: values.initialCash,
  };

  if (strategy === "random") {
    return {
      ...baseRequest,
      target_notional: values.targetNotional,
      seed: values.seed,
    };
  }

  return baseRequest;
}

export async function runBacktest(strategy: StrategyId, values: BacktestFormValues) {
  const request = buildBacktestRequest(strategy, values);

  if (strategy === "random") {
    return runRandomBacktest(request as RandomBacktestRequest);
  }

  return runBuyHoldBacktest(request as BuyHoldBacktestRequest);
}

export async function runBuyHoldBacktest(request: BuyHoldBacktestRequest) {
  const response = await apiFetch("/backtests/buy-and-hold", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(request),
  });

  return response.json() as Promise<BacktestResult>;
}
