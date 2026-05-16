from pathlib import Path
import sys
from time import perf_counter
from typing import Sequence
from pandas import DataFrame

repo_root = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(repo_root))
sys.path.insert(0, str(repo_root / "build"))

DATA_DIR = repo_root / "ml" / "data"

import engine_cpp
from ml.data_ingestion.data_source import DataSource, SOURCE


def run_buy_hold_backtest(
    tickers: Sequence[str] | None = None,
    start="2026-01-01",
    end="2026-05-01",
    source=SOURCE.YAHOO,
    distribution: dict[str, float] | None = None,
    initial_cash: float = 100000.0,
    seed: int = 42,
) -> dict:
    total_start = perf_counter()

    # Fallback for empty tickers
    if tickers is None:
        tickers = ["AAPL", "AMZN"]

    # Using data source for financial data from yfinance. Eventually should support
    ds = DataSource(data_dir=DATA_DIR)
    data_fetch_start = perf_counter()
    df: DataFrame = ds.get_ohlvc(
        tickers=tickers,
        start=start,
        end=end,
        interval="1d",
        source=source,
        auto_adjust=True,
        use_cache="refresh_missing",
    )
    data_fetch_seconds = perf_counter() - data_fetch_start

    # Convert yfinance data to cpp Ticks
    ticks = [
        engine_cpp.Tick(
            int(row.timestamp),
            str(row.ticker),
            float(row.open),
            float(row.high),
            float(row.low),
            float(row.volume),
            float(row.close),
        )
        for row in df.itertuples(index=False)
    ]

    # Ingest ticks into Data store
    result = engine_cpp.MarketDataStore.from_ticks(rows=ticks)
    data_store_stats = result.stats

    # Display ingestion stats
    print(f"Dropped rows: {data_store_stats.dropped_duplicates}")
    print(f"Dropped invalid : {data_store_stats.dropped_invalid}")
    print(f"Input rows: {data_store_stats.input_rows}")
    print(f"Kept rows: {data_store_stats.kept_rows}")
    print(f"Number of time slices: {data_store_stats.num_slices}")
    print(f"Number of assets: {data_store_stats.num_symbols}")

    # Declare market data store and symbols of assets of interest
    store: engine_cpp.MarketDataStore = result.store
    universe_ids = store.symbol_ids()

    # Declare a random strategy for now, should eventually be a parametrized strategy tunable via web UI
    strategy = engine_cpp.BuyHoldStrategy(
        universe_ids,
        # distribution=distribution,
    )

    # Declare simple portfolio
    portfolio = engine_cpp.Portfolio(initial_cash)

    # Prepare feature configuration class with empty schema (no features compputed)
    feature_config = engine_cpp.FeatureConfig(
        engine_cpp.FeatureSchema([engine_cpp.FeatureKind.Return]),
        window=20,
        enable_cross_section=False,
    )

    # Declare and run backtester
    backtester = engine_cpp.Backtester(
        store,
        strategy,
        portfolio,
        feature_config,
        0.001,
    )

    backtest_start = perf_counter()
    backtester.run()
    backtest_run_seconds = perf_counter() - backtest_start

    equity = backtester.get_equity()
    orders = backtester.get_orders()
    fills = backtester.get_fills()
    symbols = [store.get_symbol(i) for i in universe_ids]
    initial_equity = float(equity[0][1]) if equity else initial_cash
    final_equity = float(equity[-1][1]) if equity else initial_cash
    total_seconds = perf_counter() - total_start

    return {
        "run_config": {
            "tickers": list(tickers),
            "start": start,
            "end": end,
            "source": str(source),
            "distribution": distribution,
            "initial_cash": initial_cash,
            "seed": seed,
        },
        "summary": {
            "initial_equity": initial_equity,
            "final_equity": final_equity,
            "pnl": final_equity - initial_equity,
            "return_pct": (final_equity / initial_equity - 1.0) * 100.0,
            "rows": data_store_stats.kept_rows,
            "equity_points": len(equity),
            "orders": len(orders),
            "fills": len(fills),
        },
        "timing": {
            "data_fetch_seconds": data_fetch_seconds,
            "backtest_run_seconds": backtest_run_seconds,
            "total_seconds": total_seconds,
        },
        "symbols": symbols,
        "equity_curve": [
            {"timestamp": int(ts), "equity": float(value)} for ts, value in equity
        ],
        "price_curve": [
            {
                "timestamp": int(row.timestamp),
                "ticker": str(row.ticker),
                "close": float(row.close),
            }
            for row in df.itertuples(index=False)
        ],
    }


def main():
    summary = run_buy_hold_backtest()
    for key in summary.keys():
        print(f"{key} : {summary[key]}")


if __name__ == "__main__":
    main()
