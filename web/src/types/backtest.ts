
// Contains the parameters passed into a request for run_random_request
export type StrategyId = "random" | "buy_hold";

export type StrategyOption = {
  id: StrategyId;
  label: string;
};

export type BacktestFormValues = {
  selectedTickers: string[];
  start: string;
  end: string;
  initialCash: number;
  targetNotional: number;
  seed: number;
};

export type SupportedTicker = {
  symbol: string;
  name: string;
  market: string;
  sector?: string;
};

export type RandomBacktestRequest = {
  tickers: string[];
  start: string;
  end: string;
  target_notional: number;
  initial_cash: number;
  seed: number;
};

export type BuyHoldBacktestRequest = {
  tickers: string[];
  start: string;
  end: string;
  initial_cash: number;
};

export type PricePoint = {
  timestamp: number;
  ticker: string;
  close: number;
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

// Timing metrics measured by the Python backtest runner.
export type BacktestTiming = {
  data_fetch_seconds: number;
  backtest_run_seconds: number;
  total_seconds: number;
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
  timing: BacktestTiming;
  symbols: string[];
  equity_curve: EquityPoint[];
  price_curve: PricePoint[];
};

export type BacktestRun = {
  strategy: StrategyOption;
  result: BacktestResult;
};

export type OnlineSimulationRequest = {
  tickers: string[];
  lookback_minutes: number;
  interval: string;
  max_polls: number;
  poll_seconds: number;
  target_notional: number;
  initial_cash: number;
  seed: number;
};

export type OnlineSimulationSummary = {
  polls: number;
  processed_slices: number;
  orders: number;
  fills: number;
  final_equity: number;
};

export type OnlinePollResult = {
  poll: number;
  fetched_rows: number;
  new_rows: number;
  new_slices: number;
  step_results: number;
  last_seen_by_ticker: Record<string, number>;
  seconds: number;
};

export type OnlineSimulationResult = {
  run_config: Record<string, unknown>;
  summary: OnlineSimulationSummary;
  timing: {
    total_seconds: number;
  };
  symbols: string[];
  last_seen_by_ticker: Record<string, number>;
  poll_results: OnlinePollResult[];
  step_results: Array<Record<string, unknown>>;
  equity_curve: EquityPoint[];
  price_curve: PricePoint[];
};
