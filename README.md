# Trading Engine

C++ backtesting and online simulation engine for systematic trading research.

The goal is to keep the hot path in C++ while exposing enough Python bindings for notebooks, experiments, and custom strategies.

## Structure

- `cpp/core/`: event types and engine loop
- `cpp/backtest/`: historical backtester over `MarketDataStore`
- `cpp/simulation/`: online-style simulator APIs
- `cpp/data/`: CSV parsing, market-data storage, online features, cross-section features
- `cpp/portfolio/`: positions, cash, prices, equity, exposures
- `cpp/execution/`: order queue and fill simulation
- `cpp/risk/`: risk-manager interfaces and simple limits
- `cpp/strategy/`: strategy interface and built-in strategies
- `cpp/python/`: `pybind11` module exposed as `engine_cpp`
- `cpp/tests/`: C++ regression tests
- `ml/`: Python research code, notebooks, tests, and data tools

## Capabilities

- Event-driven order/fill/portfolio simulation
- Slice-wise market processing for cross-sectional workflows
- Online rolling features per asset
- Cross-sectional ranks and z-scores
- C++ and Python-defined strategies
- Optional risk-manager hooks
- Python bindings for research workflows
- CMake build with CTest coverage

## Build

From the repository root:

```bash
cmake -S cpp -B build -DPython_EXECUTABLE="$PWD/.venv/bin/python"
cmake --build build -j
ctest --test-dir build --output-on-failure
```

Run the C++ benchmark app:

```bash
./build/engine
```

Use the Python extension:

```bash
PYTHONPATH=build .venv/bin/python -c "import engine_cpp; print(engine_cpp.__version__)"
```
