from __future__ import annotations

from pathlib import Path
from typing import Dict, Iterable

import pandas as pd

from .data_source import DataSource, SOURCE
from .yahoo import validate_tickers as yahoo_validate_tickers


_DEFAULT_SOURCE = DataSource()


def load_ohlvc(
    tickers: Iterable[str],
    start: str | pd.Timestamp | None = None,
    end: str | pd.Timestamp | None = None,
    period: str | None = "10y",
    interval: str = "1d",
    auto_adjust: bool = False,
    source: str | SOURCE = SOURCE.YAHOO,
) -> pd.DataFrame:
    return _DEFAULT_SOURCE.load_ohlvc_raw(
        tickers=tickers,
        start=start,
        end=end,
        period=period,
        interval=interval,
        source=source,
        auto_adjust=auto_adjust,
    )


def clean_ohlvc(df: pd.DataFrame) -> pd.DataFrame:
    return _DEFAULT_SOURCE._clean_ohlvc(df)


def save_to_ml_data(filename: str, data: pd.DataFrame) -> None:
    base = Path("/Users/nicolashoubouyan/Projects/engine/ml/data")
    base.mkdir(parents=True, exist_ok=True)
    data.to_csv(base / filename, index=False)


def split_by_asset(df: pd.DataFrame) -> Dict[str, pd.DataFrame]:
    clean = clean_ohlvc(df)
    out: Dict[str, pd.DataFrame] = {}
    for ticker in sorted(clean["ticker"].unique()):
        ticker_df = (
            clean.loc[clean["ticker"] == ticker]
            .sort_values("timestamp")
            .set_index("timestamp")
        )
        out[ticker] = ticker_df
    return out
