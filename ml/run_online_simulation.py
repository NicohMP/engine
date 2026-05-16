from pathlib import Path
import sys
from time import sleep
from time import perf_counter
from typing import Sequence
import pandas as pd
from pandas import DataFrame

repo_root = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(repo_root))
sys.path.insert(0, str(repo_root / "build"))

DATA_DIR = repo_root / "ml" / "data"

import engine_cpp
from ml.data_ingestion.data_source import DataSource, SOURCE


def normalize_tickers(tickers: Sequence[str]) -> list[str]:
    symbols = []
    seen = set()

    for ticker in tickers:
        symbol = str(ticker).strip().upper()
        if symbol and symbol not in seen:
            symbols.append(symbol)
            seen.add(symbol)

    if not symbols:
        raise ValueError("tickers must be non-empty")

    return symbols

def build_symbol_maps(tickers: Sequence[str]) -> tuple[dict[str, int], dict[int, str]]:
    symbols = normalize_tickers(tickers)
    symbol_to_id = {ticker: idx for idx, ticker in enumerate(symbols)}
    id_to_symbol = {idx: ticker for ticker, idx in symbol_to_id.items()}
    return symbol_to_id, id_to_symbol

def row_to_market_event(row, symbol_to_id: dict[str, int]):
    ticker = str(row.ticker).strip().upper()
    if ticker not in symbol_to_id:
        raise KeyError(f"Unknown ticker in dataframe: {ticker}")

    return engine_cpp.MarketEvent(
        int(row.timestamp),
        int(symbol_to_id[ticker]),
        float(row.open),
        float(row.high),
        float(row.low),
        float(row.volume),
        float(row.close),
    )

def dataframe_to_market_event_slices(df: pd.DataFrame, symbol_to_id: dict[str, int]):
    ordered = df.copy().sort_values(["timestamp", "ticker"]).reset_index(drop=True)
    slices: list[tuple[int, list[engine_cpp.MarketEvent]]] = []

    for timestamp, group in ordered.groupby("timestamp", sort=True):
        events = [
            row_to_market_event(row, symbol_to_id)
            for row in group.itertuples(index=False)
            if str(row.ticker).strip().upper() in symbol_to_id
        ]

        if events:
            slices.append((int(timestamp), events))

    return slices

def fetch_recent_data(
    ds: DataSource,
    tickers: Sequence[str],
    lookback_minutes: int,
    interval: str = "1m",
    source=SOURCE.YAHOO,
) -> DataFrame:
    now = pd.Timestamp.now(tz="UTC")
    start = now - pd.Timedelta(minutes=lookback_minutes)

    return ds.get_ohlvc(
        tickers=tickers,
        start=start,
        end=now,
        interval=interval,
        source=source,
        auto_adjust=True,
        use_cache="refresh_missing",
    )

def filter_unseen_rows(
    df: DataFrame,
    last_seen_by_ticker: dict[str, int],
) -> DataFrame:
    keep = []

    for row in df.itertuples(index=False):
        ticker = str(row.ticker).strip().upper()
        timestamp = int(row.timestamp)
        last_seen = last_seen_by_ticker.get(ticker, -1)
        keep.append(timestamp > last_seen)

    return df.loc[keep].copy()

def update_last_seen(
    df: DataFrame,
    last_seen_by_ticker: dict[str, int],
) -> dict[str, int]:
    for ticker in list(last_seen_by_ticker.keys()):
        ticker_df: pd.DataFrame = df[df["ticker"].astype(str).str.upper() == ticker]
        if ticker_df.empty:
            continue

        latest_in_df = int(ticker_df["timestamp"].max())
        if latest_in_df > last_seen_by_ticker[ticker]:
            last_seen_by_ticker[ticker] = latest_in_df

    return last_seen_by_ticker

def create_random_simulator(
    universe_ids: list[int],
    target_notional: float,
    initial_cash: float,
    seed: int,
) -> engine_cpp.Simulator:
    strategy: engine_cpp.Strategy = engine_cpp.RandomStrategy(
        universe_ids=universe_ids,
        target_notional=target_notional,
        seed=seed,
    )
    portfolio: engine_cpp.Portfolio = engine_cpp.Portfolio(
        init_cash=initial_cash
    )
    feat_config: engine_cpp.FeatureConfig = engine_cpp.FeatureConfig(
        schema=engine_cpp.FeatureSchema([engine_cpp.FeatureKind.Return]),
        window=20,
        enable_cross_section=False,
    )
    sim: engine_cpp.Simulator = engine_cpp.Simulator(
        universe=universe_ids,
        strategy=strategy,
        portfolio=portfolio,
        feature_config=feat_config,
    )
    return sim


def step_slices(
    simulator: engine_cpp.Simulator,
    slices: list[tuple[int, list[engine_cpp.MarketEvent]]],
) -> tuple[list[dict], list[dict]]:
    step_results = []
    equity_curve = []

    for timestamp, events in slices:
        result = simulator.step_timestamp(events)
        step_results.append(
            {
                "timestamp": result.timestamp,
                "new_orders": len(result.new_orders),
                "new_fills": len(result.new_fills),
                "equity_after": float(result.equity_after),
                "total_orders": int(result.total_orders),
                "total_fills": int(result.total_fills),
            }
        )
        equity_curve.append(
            {"timestamp": int(timestamp), "equity": float(result.equity_after)}
        )

    return step_results, equity_curve


def run_random_online_simulation(
    tickers: Sequence[str] | None = None,
    lookback_minutes: int = 120,
    interval: str = "1m",
    max_slices: int | None = None,
    source=SOURCE.YAHOO,
    target_notional: float = 1000.0,
    initial_cash: float = 100000.0,
    seed: int = 42,
) -> dict:
    total_start = perf_counter()

    if tickers is None:
        tickers = ["AAPL", "AMZN"]

    tickers = normalize_tickers(tickers)
    symbol_to_id, id_to_symbol = build_symbol_maps(tickers)
    universe_ids = list(id_to_symbol.keys())

    ds = DataSource(data_dir=DATA_DIR)

    df = fetch_recent_data(
        ds=ds,
        tickers=tickers,
        lookback_minutes=lookback_minutes,
        interval=interval,
        source=source,
    )

    slices = dataframe_to_market_event_slices(df, symbol_to_id)

    if max_slices is not None:
        slices = slices[:max_slices]

    simulator = create_random_simulator(
        universe_ids=universe_ids,
        target_notional=target_notional,
        initial_cash=initial_cash,
        seed=seed,
    )

    step_results, equity_curve = step_slices(simulator, slices)

    equity = simulator.get_equity()
    final_equity = float(equity[-1][1]) if equity else initial_cash
    total_seconds = perf_counter() - total_start

    return {
        "run_config": {
            "tickers": tickers,
            "interval": interval,
            "lookback_minutes": lookback_minutes,
            "source": str(source),
            "target_notional": target_notional,
            "initial_cash": initial_cash,
            "seed": seed,
            "max_slices": max_slices,
        },
        "summary": {
            "rows": len(df),
            "slices": len(slices),
            "orders": len(simulator.get_orders()),
            "fills": len(simulator.get_fills()),
            "final_equity": final_equity,
        },
        "timing": {
            "total_seconds": total_seconds,
        },
        "symbols": tickers,
        "step_results": step_results,
        "equity_curve": equity_curve,
        "price_curve": [
            {
                "timestamp": int(row.timestamp),
                "ticker": str(row.ticker),
                "close": float(row.close),
            }
            for row in df.itertuples(index=False)
        ],
    }


def poll_random_online_simulation(
    tickers: Sequence[str] | None = None,
    lookback_minutes: int = 120,
    interval: str = "1m",
    max_polls: int = 5,
    poll_seconds: float = 10.0,
    source=SOURCE.YAHOO,
    target_notional: float = 1000.0,
    initial_cash: float = 100000.0,
    seed: int = 42,
) -> dict:
    total_start = perf_counter()

    if tickers is None:
        tickers = ["AAPL", "AMZN"]

    tickers = normalize_tickers(tickers)
    symbol_to_id, id_to_symbol = build_symbol_maps(tickers)
    universe_ids = list(id_to_symbol.keys())
    last_seen_by_ticker = {ticker: -1 for ticker in tickers}

    ds = DataSource(data_dir=DATA_DIR)
    simulator = create_random_simulator(
        universe_ids=universe_ids,
        target_notional=target_notional,
        initial_cash=initial_cash,
        seed=seed,
    )

    all_step_results = []
    equity_curve = []
    price_curve = []
    poll_results = []

    for poll_index in range(max_polls):
        poll_start = perf_counter()
        df = fetch_recent_data(
            ds=ds,
            tickers=tickers,
            lookback_minutes=lookback_minutes,
            interval=interval,
            source=source,
        )
        new_df = filter_unseen_rows(df, last_seen_by_ticker)
        slices = dataframe_to_market_event_slices(new_df, symbol_to_id)
        step_results, new_equity_points = step_slices(simulator, slices)

        all_step_results.extend(step_results)
        equity_curve.extend(new_equity_points)
        price_curve.extend(
            {
                "timestamp": int(row.timestamp),
                "ticker": str(row.ticker),
                "close": float(row.close),
            }
            for row in new_df.itertuples(index=False)
        )
        update_last_seen(new_df, last_seen_by_ticker)

        poll_results.append(
            {
                "poll": poll_index + 1,
                "fetched_rows": len(df),
                "new_rows": len(new_df),
                "new_slices": len(slices),
                "step_results": len(step_results),
                "last_seen_by_ticker": dict(last_seen_by_ticker),
                "seconds": perf_counter() - poll_start,
            }
        )

        if poll_index < max_polls - 1 and poll_seconds > 0:
            sleep(poll_seconds)

    equity = simulator.get_equity()
    final_equity = float(equity[-1][1]) if equity else initial_cash
    total_seconds = perf_counter() - total_start

    return {
        "run_config": {
            "tickers": tickers,
            "interval": interval,
            "lookback_minutes": lookback_minutes,
            "source": str(source),
            "target_notional": target_notional,
            "initial_cash": initial_cash,
            "seed": seed,
            "max_polls": max_polls,
            "poll_seconds": poll_seconds,
        },
        "summary": {
            "polls": max_polls,
            "processed_slices": len(all_step_results),
            "orders": len(simulator.get_orders()),
            "fills": len(simulator.get_fills()),
            "final_equity": final_equity,
        },
        "timing": {
            "total_seconds": total_seconds,
        },
        "symbols": tickers,
        "last_seen_by_ticker": last_seen_by_ticker,
        "poll_results": poll_results,
        "step_results": all_step_results,
        "equity_curve": equity_curve,
        "price_curve": price_curve,
    }



def main():
    result = poll_random_online_simulation(max_polls=3, poll_seconds=0.0)
    print("summary:", result["summary"])
    for poll in result["poll_results"]:
        print(poll)


if __name__ == "__main__":
    main()
