from __future__ import annotations

from typing import Iterable

import pandas as pd
import yfinance as yf


_PRICE_LEVEL_NAMES = {"Open", "High", "Low", "Close", "Adj Close", "Volume"}


def validate_tickers(tickers: Iterable[str]) -> tuple[list[str], list[str]]:
    valid: list[str] = []
    invalid: list[str] = []

    for symbol in [str(t).strip().upper() for t in tickers]:
        try:
            hist = yf.Ticker(symbol).history(period="1mo")
        except Exception:
            invalid.append(symbol)
            continue

        if hist is None or hist.empty:
            invalid.append(symbol)
        else:
            valid.append(symbol)

    return valid, invalid


def _detect_ticker_level(columns: pd.MultiIndex, tickers: list[str]) -> int:
    if columns.nlevels != 2:
        raise ValueError(
            f"Expected 2-level MultiIndex from yfinance, got {columns.nlevels} levels."
        )

    ticker_set = set(tickers)
    level0 = {str(v) for v in columns.get_level_values(0).unique()}
    level1 = {str(v) for v in columns.get_level_values(1).unique()}

    level0_price_hits = len(level0 & _PRICE_LEVEL_NAMES)
    level1_price_hits = len(level1 & _PRICE_LEVEL_NAMES)
    level0_ticker_hits = len(level0 & ticker_set)
    level1_ticker_hits = len(level1 & ticker_set)

    if level0_price_hits > level1_price_hits:
        return 1
    if level1_price_hits > level0_price_hits:
        return 0
    if level0_ticker_hits > level1_ticker_hits:
        return 0
    return 1


def load_ohlvc(
    *,
    tickers: Iterable[str],
    start: str | pd.Timestamp | None = None,
    end: str | pd.Timestamp | None = None,
    period: str | None = None,
    interval: str = "1d",
    auto_adjust: bool = False,
) -> pd.DataFrame:
    """
    Load Yahoo OHLCV data into the canonical pre-clean flat schema:
    columns = ['timestamp', 'ticker', 'open', 'high', 'low', 'close', 'volume'].
    """
    symbols = [str(t).strip().upper() for t in tickers]
    if not symbols:
        raise ValueError("tickers must be a non-empty iterable of symbols")

    valid, invalid = validate_tickers(symbols)
    if not valid:
        raise RuntimeError(f"No valid Yahoo tickers found in request: {symbols}")

    download_kwargs = {
        "tickers": valid,
        "interval": interval,
        "auto_adjust": auto_adjust,
        "progress": False,
        "actions": False,
    }
    if period is not None:
        download_kwargs["period"] = period
    else:
        if start is None or end is None:
            raise ValueError("Either provide period or both start and end")
        download_kwargs["start"] = pd.to_datetime(start)
        download_kwargs["end"] = pd.to_datetime(end) + pd.Timedelta(days=1)

    raw = yf.download(**download_kwargs)
    if raw is None or raw.empty:
        raise RuntimeError(
            f"Yahoo returned no OHLCV data for valid tickers: {valid}"
        )

    if isinstance(raw.columns, pd.MultiIndex):
        ticker_level = _detect_ticker_level(raw.columns, valid)
        try:
            stacked = raw.stack(level=ticker_level, future_stack=True)
        except TypeError:
            stacked = raw.stack(level=ticker_level)
        df = stacked.rename_axis(index=["date", "ticker"]).reset_index()
    else:
        df = raw.reset_index()
        df["ticker"] = valid[0]
        if "Date" in df.columns:
            df = df.rename(columns={"Date": "date"})

    df.columns = [str(c).lower().replace(" ", "_") for c in df.columns]
    if "date" not in df.columns:
        for candidate in ("datetime", "index", "level_0"):
            if candidate in df.columns:
                df = df.rename(columns={candidate: "date"})
                break
    if "ticker" not in df.columns:
        for candidate in ("symbol", "level_1"):
            if candidate in df.columns:
                df = df.rename(columns={candidate: "ticker"})
                break

    dt = pd.to_datetime(df["date"], utc=True, errors="coerce")
    epoch = pd.Timestamp("1970-01-01", tz="UTC")
    df["timestamp"] = (dt - epoch) // pd.Timedelta(milliseconds=1)

    keep = ["timestamp", "ticker", "open", "high", "low", "close", "volume"]
    missing = [col for col in keep if col not in df.columns]
    if missing:
        raise RuntimeError(
            f"Yahoo loader missing canonical OHLCV columns: {missing}"
        )

    out = df[keep].copy()
    if invalid:
        out.attrs["invalid_tickers"] = invalid
    return out
