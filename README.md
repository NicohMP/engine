# Trading Engine

C++ backtesting and online simulation engine for systematic trading research.

The goal is to keep the hot path in C++ while exposing enough Python bindings for notebooks, experiments, and custom strategies.

## Run locally

From the repository root:

```bash
./scripts/setup.sh
```

This creates the Python environment, installs Python and frontend dependencies,
and builds the C++ engine and `engine_cpp` bindings. It requires Python 3.12,
CMake, and npm.

Start the API and web UI:

```bash
./scripts/start.sh
```

Then open `http://localhost:5173`.

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



## Docker UI

For the simplest browser demo, install Docker Desktop, then run this from the
repository root:

```bash
docker compose up --build
```

Then open:

- Web UI: `http://localhost:5173`
- API docs: `http://localhost:8000/docs`
- API health: `http://localhost:8000/health`

Stop everything with:

```bash
docker compose down
```
