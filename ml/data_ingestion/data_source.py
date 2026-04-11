from __future__ import annotations

from enum import StrEnum
from pathlib import Path
from typing import Callable, Dict, Iterable, List
import re

import pandas as pd

from . import yahoo


class SOURCE(StrEnum):
    YAHOO = "yahoo"


OHLCVLoader = Callable[..., pd.DataFrame]

_MS_MIN_2000 = 946684800000
_MS_MAX_2100 = 4102444800000


class DataSource:
    """
    Central authority for ingestion.

    Responsibilities:
    - Select the correct source-specific loader.
    - Enforce the canonical pre-clean OHLCV schema expected from loaders.
    - Clean and validate OHLCV data into a canonical flat dataframe.
    """

    REQUIRED_OHLCV_COLUMNS = [
        "timestamp",
        "ticker",
        "open",
        "high",
        "low",
        "close",
        "volume",
    ]

    DEFAULT_OHLCV_LOADERS: Dict[str, OHLCVLoader] = {
        SOURCE.YAHOO: yahoo.load_ohlvc, }

    DEFAULT_OHLCV_PERIOD_BY_INTERVAL = {
        "1d": "25y", "1h": "730d", "1m": "30d", }

    DEFAULT_OHLCV_ROWS = 5000

    def __init__(self, data_dir: str | Path = "data_cache") -> None:
        self.data_dir = Path(data_dir)
        self.data_dir.mkdir(parents=True, exist_ok=True)
        self._ohlcv_loaders: Dict[str, OHLCVLoader] = dict(
            self.DEFAULT_OHLCV_LOADERS)

    def register_ohlcv_loader(self, source: str | SOURCE, loader: OHLCVLoader) -> None:
        source_name = _normalize_source(source)
        self._ohlcv_loaders[source_name] = loader

    def load_ohlvc_raw(
        self,
        *,
        tickers: Iterable[str],
        start: str | pd.Timestamp | None = None,
        end: str | pd.Timestamp | None = None,
        period: str | None = None,
        interval: str = "1d",
        source: str | SOURCE = SOURCE.YAHOO,
        auto_adjust: bool = False,
    ) -> pd.DataFrame:
        source_name = _normalize_source(source)
        try:
            loader = self._ohlcv_loaders[source_name]
        except KeyError as exc:
            raise ValueError(
                f"No OHLCV loader registered for source '{source_name}'") from exc

        raw = loader(
            tickers=tickers,
            start=start,
            end=end,
            period=period,
            interval=interval,
            auto_adjust=auto_adjust,
        )
        self._validate_ohlcv_loader_output(raw)
        return raw

    def get_ohlvc(
        self,
        *,
        tickers: Iterable[str],
        start: str | pd.Timestamp | None = None,
        end: str | pd.Timestamp | None = None,
        period: str | None = None,
        interval: str = "1d",
        source: str | SOURCE = SOURCE.YAHOO,
        auto_adjust: bool = False,
        use_cache: bool = True,
    ) -> pd.DataFrame:
        """Retriveve financial data for an asset, from a specified source.
        If `use_cache` is True, for each (ticker,interval,source) in the list, the method first checks if it's already stored in the cache.
        If so the data come from the cache, otherwise the source is called. 

        Args:
            tickers (Iterable[str]): List of assets of interest
            start (str | pd.Timestamp | None, optional): Starting date of period of interest. Defaults to None.
            end (str | pd.Timestamp | None, optional): End date of period of interest. Defaults to None.
            period (str | None, optional): Period of interest. Defaults to None.
            interval (str, optional): Interval of time between contiguous info . Defaults to "1d".
            source (str | SOURCE, optional): Source of info. Defaults to SOURCE.YAHOO (yfinance).
            auto_adjust (bool, optional): Wether to also retrieve the auto-adjusted close price. Defaults to False.

        Returns:
            pd.DataFrame: Dataframe with requested open,high,low,volume,close prices over the requested period 
        """
        if interval not in self.DEFAULT_OHLCV_PERIOD_BY_INTERVAL:
            raise ValueError(
                f"Interval {interval} not supported by DataSource")
        symbols = [str(ticker).strip().upper() for ticker in tickers]
        symbols = [ticker for ticker in symbols if ticker]
        if not symbols:
            raise ValueError("tickers must be a non-empty iterable of symbols")

        hits: list[pd.DataFrame] = []
        missing: list[str] = []
        can_use_cache = use_cache and period is None and start is not None and end is not None

        if can_use_cache:
            for ticker in symbols:
                cached = self._load_ohlvc_from_cache(
                    ticker=ticker,
                    source=source,
                    interval=interval,
                    start=start,
                    end=end,
                )
                if cached is None:
                    missing.append(ticker)
                else:
                    hits.append(cached)
        else:
            missing = symbols

        if missing:
            raw = self.load_ohlvc_raw(
                tickers=missing,
                start=start,
                end=end,
                period=period,
                interval=interval,
                source=source,
                auto_adjust=auto_adjust,
            )
            fresh = self._clean_ohlvc(raw)
            hits.append(fresh)

        if not hits:
            return pd.DataFrame(columns=self.REQUIRED_OHLCV_COLUMNS)

        return (
            pd.concat(hits, ignore_index=True)
            .sort_values(["timestamp", "ticker"])
            .reset_index(drop=True)
        )

    def update_ohlvc(self, tickers: List[str], source: SOURCE, interval: str):
        if interval not in self.DEFAULT_OHLCV_PERIOD_BY_INTERVAL:
            raise ValueError(
                f"Interval {interval} not supported by DataSource")
        tickers = [str(ticker).strip().upper() for ticker in tickers]
        tickers = [ticker for ticker in tickers if ticker]
        if not tickers:
            raise ValueError("tickers must be a non-empty iterable of symbols")

        source_name = _normalize_source(source)

        try:
            loader = self._ohlcv_loaders[source_name]
        except KeyError as exc:
            raise ValueError(
                f"No OHLCV loader registered for source '{source_name}'") from exc

        raw = loader(
            tickers=tickers, period=self.DEFAULT_OHLCV_PERIOD_BY_INTERVAL[interval], interval=interval)
        clean = self._clean_ohlvc(raw)

        for ticker in tickers:
            path: Path = self._cache_path(
                ticker, source_name, "ohlvc", interval)
            ticker_clean = clean[clean["ticker"] == ticker.upper()]
            ticker_clean = ticker_clean.sort_values(
                "timestamp").tail(self.DEFAULT_OHLCV_ROWS)
            path.parent.mkdir(parents=True, exist_ok=True)
            ticker_clean.to_parquet(path, index=False)
        return clean

    def to_panel(self, df: pd.DataFrame) -> pd.DataFrame:
        """
        Convert canonical flat OHLCV data into a (timestamp, ticker) MultiIndex panel.
        """
        self._validate_canonical_ohlcv(df)
        panel = (
            df.copy()
            .set_index(["timestamp", "ticker"])
            .sort_index()
        )
        return panel

    def _cache_path(self, ticker: str, source: str | SOURCE, dataset_type: str, interval: str,) -> Path:
        return (
            self.data_dir
            / _normalize_source(source)
            / _safe_path_part(dataset_type).lower()
            / _safe_path_part(interval).lower()
            / f"{_safe_path_part(ticker)}.parquet"
        )

    @staticmethod
    def _timestamp_to_ms(ts: str | pd.Timestamp) -> int:
        dt = pd.to_datetime(ts, utc=True)
        epoch = pd.Timestamp("1970-01-01", tz="UTC")
        return int((dt - epoch) // pd.Timedelta(milliseconds=1))

    @staticmethod
    def _normalize_timestamp_ms(ts: pd.Series) -> pd.Series:
        raw = pd.to_numeric(ts, errors="coerce")
        if raw.notna().sum() == 0:
            return raw

        candidates = {
            "ms": raw,
            "from_s": raw * 1_000.0,
            "from_us": raw / 1_000.0,
            "from_ns": raw / 1_000_000.0,
            "from_bug_1e15": raw * 1_000_000_000.0,
        }

        def valid_ratio(candidate: pd.Series) -> float:
            return candidate.between(_MS_MIN_2000, _MS_MAX_2100).mean()

        best_name = max(
            candidates, key=lambda name: valid_ratio(candidates[name]))
        best = candidates[best_name]
        if valid_ratio(best) >= 0.95:
            return best.round()
        return raw

    def _validate_ohlcv_loader_output(self, df: pd.DataFrame) -> None:
        if not isinstance(df, pd.DataFrame):
            raise TypeError("OHLCV loader must return a pandas.DataFrame")
        missing = [
            col for col in self.REQUIRED_OHLCV_COLUMNS if col not in df.columns]
        if missing:
            raise ValueError(
                f"OHLCV loader output missing required columns: {missing}"
            )

    def _validate_canonical_ohlcv(self, df: pd.DataFrame) -> None:
        self._validate_ohlcv_loader_output(df)
        if df.empty:
            raise ValueError("Canonical OHLCV dataframe is empty")
        if df.duplicated(["timestamp", "ticker"]).any():
            raise ValueError(
                "Canonical OHLCV dataframe contains duplicate (timestamp, ticker) rows")

    def _clean_ohlvc(self, df: pd.DataFrame) -> pd.DataFrame:
        """
        Canonical OHLCV cleaning step applied to every source.
        Expects columns:
        ['timestamp', 'ticker', 'open', 'high', 'low', 'close', 'volume'].
        """
        self._validate_ohlcv_loader_output(df)

        out = df.loc[:, self.REQUIRED_OHLCV_COLUMNS].copy()
        out["timestamp"] = self._normalize_timestamp_ms(out["timestamp"])

        num_cols = ["open", "high", "low", "close", "volume"]
        out[num_cols] = out[num_cols].apply(pd.to_numeric, errors="coerce")
        out["timestamp"] = pd.to_numeric(out["timestamp"], errors="coerce")
        out["ticker"] = out["ticker"].astype(str).str.strip().str.upper()

        out = out.dropna(
            subset=["timestamp", "ticker", "open",
                    "high", "low", "close", "volume"]
        )
        out["timestamp"] = out["timestamp"].round().astype("int64")

        duplicate_mask = out.duplicated(["timestamp", "ticker"])
        duplicate_count = int(duplicate_mask.sum())
        if duplicate_count > max(10, len(out) // 100):
            raise ValueError(
                "Timestamp normalization created too many duplicate (timestamp, ticker) rows "
                f"({duplicate_count} duplicates across {len(out)} rows). "
                "This usually means timestamp precision was lost before cleaning."
            )

        out = (
            out.sort_values(["timestamp", "ticker"])
            .drop_duplicates(["timestamp", "ticker"])
            .reset_index(drop=True)
        )
        self._validate_canonical_ohlcv(out)
        return out

    def _load_ohlvc_from_cache(
        self,
        ticker: str,
        source: SOURCE,
        interval: str,
        start: pd.Timestamp,
        end: pd.Timestamp,
    ) -> pd.DataFrame | None:
        path = self._cache_path(ticker, source, "ohlvc", interval)

        if not path.is_file() or path.stat().st_size == 0:
            return None

        try:
            cached = pd.read_parquet(path)
        except Exception:
            return None

        cached = self._clean_ohlvc(cached)
        cached = cached[cached["ticker"] == ticker.upper()]

        if cached.empty:
            return None

        start_ms = self._timestamp_to_ms(start)
        end_ms = self._timestamp_to_ms(end)

        if cached["timestamp"].min() > start_ms:
            return None

        if cached["timestamp"].max() < end_ms:
            return None

        return cached[
            (cached["timestamp"] >= start_ms)
            & (cached["timestamp"] <= end_ms)
        ].copy()


def _safe_path_part(value: object) -> str:
    text = str(value).strip().upper()
    text = re.sub(r"[^A-Z0-9._=-]+", "_", text)
    return text


def _normalize_source(source: str | SOURCE) -> str:
    if isinstance(source, SOURCE):
        return source.value
    return str(source).strip().lower()
