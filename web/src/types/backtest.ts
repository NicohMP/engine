
// Contains the parameters passed into a request for run_random_request
export type RandomBacktestRequest = {
  tickers: string[];
  start: string;
  end: string;
  target_notional: number;
  initial_cash: number;
  seed: number;
};


// TypeScript type describing the "summary" object returned by POST /backtests/random.
export type BacktestSummary = {
  initial_equity: number;
  final_equity: number;
  pnl: number;
  return_pct: number;
  rows: number;
  equity_points: number;
  orders: number;
  fills: number;
};

// One point in the portfolio equity curve.
export type EquityPoint = {
  timestamp: number;
  equity: number;
};

// TypeScript type describing the full JSON response from POST /backtests/random.
export type BacktestResult = {
  run_config: Record<string, unknown>;
  summary: BacktestSummary;
  symbols: string[];
  equity_curve: EquityPoint[];
};
