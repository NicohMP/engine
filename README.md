# Trading Engine

Event-driven backtesting engine with a C++ core and Python research interface.

The C++ side owns the simulation plumbing: market/order/fill events, portfolio accounting, execution, risk checks, strategy dispatch, and the backtest event loop. Python is used for research workflows, notebooks, feature engineering, strategy experiments, risk experiments, analysis, and plotting.

## Layout

- `cpp/backtest/`: event types and `Backtester`
- `cpp/portfolio/`: `Portfolio` and `Position`
- `cpp/execution/`: execution engine and order filling
- `cpp/strategy/`: strategy base class and C++ strategies
- `cpp/risk/`: risk-manager base class and C++ risk managers
- `cpp/python/`: `pybind11` bindings exposed as `engine_cpp`
- `cpp/app/`: C++ executable entry point
- `ml/`: Python research code, notebooks, data tooling, and strategy experiments

## C++ Engine

The backtester processes `MarketEvent`, `OrderEvent`, and `FillEvent` objects through an event queue.

On each market event:

1. Portfolio prices are updated.
2. The strategy proposes orders.
3. The optional risk manager accepts or rejects the order batch.
4. The execution engine receives accepted orders and creates fills.
5. The portfolio updates positions, cash, and equity.

Strategies and risk managers are interface-based:

- `Strategy` has C++ implementations such as `RandomStrategy`
- `RiskManagerBase` has C++ implementations such as `SimpleRiskManager`
- both can also be subclassed from Python through the bindings

## Python Bindings

The project builds a Python extension module named `engine_cpp` with `pybind11`.

Python can create C++ events, portfolios, strategies, risk managers, and backtesters:

```python
import engine_cpp

mevents = [
    engine_cpp.MarketEvent(1, "AAPL", 100.0, 101.0, 99.0, 1000.0, 100.0)
]

portfolio = engine_cpp.Portfolio(
    [engine_cpp.Position("AAPL", 0.0)],
    2000.0,
)

strategy = engine_cpp.RandomStrategy(["AAPL"], 200.0, seed=123)

bt = engine_cpp.Backtester(
    mevents,
    strategy,
    portfolio,
    fee_rate=0.00075,
)

bt.run()
equity = bt.get_equity()
orders = bt.get_orders()
fills = bt.get_fills()
```

Python-defined strategies and risk managers can also be passed into the C++ backtester:

```python
class AlwaysBuy(engine_cpp.Strategy):
    def __init__(self):
        super().__init__()

    def on_market_event(self, mevent, portfolio):
        return [
            engine_cpp.OrderEvent(
                0,
                mevent.ticker,
                engine_cpp.TradeSide.Buy,
                1.0,
            )
        ]


class AcceptAllRisk(engine_cpp.RiskManagerBase):
    def __init__(self):
        super().__init__()

    def accept_order_batch(self, orders, portfolio):
        return orders
```

## Build

Configure and build from the repository root:

```bash
cmake -S cpp -B build -DPython_EXECUTABLE=$(pwd)/.env/bin/python
cmake --build build
```

Run the C++ executable:

```bash
./build/engine
```

Use the Python module from the build directory:

```bash
PYTHONPATH=build .env/bin/python -c "import engine_cpp; print(engine_cpp.add(2, 3))"
```

In notebooks, add the build directory to `sys.path` before importing:

```python
import sys
sys.path.insert(0, "/Users/nicolashoubouyan/Projects/engine/build")

import engine_cpp
```
