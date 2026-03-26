# Trading Engine Stage 1

This project is a single-symbol backtesting mini-engine in C++17.

Current Stage 1 pipeline:

1. Load market data from CSV into `std::vector<Tick>`.
2. Run a strategy on each tick.
3. Convert actions to trades using a simple execution model.
4. Update position, cash, and equity.
5. Compute summary statistics from the equity curve.

## File Layout

- Domain types
  - `tick.hpp`: market data row (`Tick`)
  - `trading_types.hpp`: `Action`, `Trade`, enums
  - `position.hpp`: single-symbol position state
  - `account_state.hpp`: account snapshot (`cash`, `equity`)
- Strategy contract and implementations
  - `strategy.hpp`: abstract strategy interface
  - `strategies/sma_strategy.hpp/.cpp`: SMA crossover strategy
- Engine
  - `backtester.hpp/.cpp`: simulation loop and accounting
- Data ingestion
  - `parse_csv.hpp/.cpp`: legacy CSV parser + kline parser/conversion
- Analytics/reporting
  - `analytics.hpp/.cpp`: indicators + `BacktestReport` computation/display
- Wiring
  - `main.cpp`: creates strategy, runs backtest, prints report

## Strategy Contract

`Strategy::on_tick(const Tick&, const Position&, const AccountState&) -> Action`

- Input:
  - current tick
  - current position (for the traded symbol)
  - current account snapshot
- Output (`Action`):
  - `Buy` / `Sell` with quantity
  - `Hold` with no quantity

Contract expectation:

- `Buy` and `Sell` must carry a valid positive quantity.
- `Hold` should carry no quantity (`std::nullopt`).

## Execution Model (Stage 1)

The backtester uses a simple synchronous fill model:

- Signal is produced on the current tick.
- Trade is filled on the same tick at `tick.close`.
- No slippage and no transaction fees.

Accounting update rules:

- Buy:
  - `position += quantity`
  - `cash -= quantity * price`
- Sell:
  - `position -= quantity`
  - `cash += quantity * price`

## Equity and PnL Interpretation

Stage 1 tracks total account value as:

`equity = cash + position_quantity * current_price`

where `current_price` is the current tick close.

The equity curve is stored as:

`std::vector<std::pair<int64_t, double>>` (`timestamp`, `equity`).

## Summary Metrics

`BacktestReport` currently includes:

- final equity
- total return
- volatility (from period returns)
- max drawdown
- number of trades

## Known Scope Limits

- Single symbol per backtest instance.
- No fees/slippage/latency model.
- No multi-asset portfolio accounting yet.

## Next Generalization Point

The main extension path is multi-symbol support:

- replace single `Position` with a per-symbol position container
- keep strategy input as a snapshot abstraction
- keep execution/accounting logic in backtester layer
