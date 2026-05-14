import { apiFetch } from "./client";
import type {
  BacktestResult,
  BuyHoldBacktestRequest,
  RandomBacktestRequest,
} from "../types/backtest";


export async function runRandomBacktest(request: RandomBacktestRequest) {
  const response = await apiFetch("/backtests/random", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(request),
  });

  return response.json() as Promise<BacktestResult>;
}

export async function runBuyHoldBacktest(request: BuyHoldBacktestRequest) {
  const response = await apiFetch("/backtests/buy-and-hold", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(request),
  });

  return response.json() as Promise<BacktestResult>;
}
