from __future__ import annotations

from enum import StrEnum
from pathlib import Path
from typing import Callable, Dict, Iterable, List, Literal
import re
from dataclasses import dataclass

import pandas as pd

from . import yahoo


class SOURCE(StrEnum):
    YAHOO = "yahoo"


@dataclass(frozen=True)
class MissingRange:
    start_ms: int
    end_ms: int


@dataclass(frozen=True)
class Coverage:
    ticker: str
    source: str
    start_ms: int | None
    end_ms: int | None
    rows: int
    missing_ranges: tuple[MissingRange, ...]
    # reason: str | None = None


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
        SOURCE.YAHOO: yahoo.load_ohlvc,
    }

    DEFAULT_OHLCV_PERIOD_BY_INTERVAL = {
        "1d": "25y",
        "1h": "730d",
        "1m": "30d",
    }

    DEFAULT_OHLCV_ROWS = 5000

    def __init__(self, data_dir: str | Path = "data_cache") -> None:
        self.data_dir = Path(data_dir)
        self.data_dir.mkdir(parents=True, exist_ok=True)
        self._ohlcv_loaders: Dict[str, OHLCVLoader] = dict(self.DEFAULT_OHLCV_LOADERS)

    def fetch_ohlvc_raw(
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
                f"No OHLCV loader registered for source '{source_name}'"
            ) from exc

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

    def read_ohlvc(
        self,
        ticker: str,
        source: SOURCE,
        interval: str,
        start: pd.Timestamp,
        end: pd.Timestamp,
        require_coverage: bool = True,
    ) -> pd.DataFrame | None:
        path = self._cache_path(ticker, source, "ohlvc", interval)

        # If no file return None
        if not path.is_file() or path.stat().st_size == 0:
            return None
        # If empty file return None
        try:
            cached = pd.read_parquet(path)
        except Exception:
            return None

        cached = self._clean_ohlvc(cached)
        cached = cached[cached["ticker"] == ticker.upper()]
        # If ticker not in file return None
        if cached.empty:
            return None

        start_ms = self._timestamp_to_ms(start)
        end_ms = self._timestamp_to_ms(end)

        if require_coverage:
            slack_ms = _coverage_slack_ms(interval)
            if (cached["timestamp"].min() - start_ms > slack_ms) or (
                end_ms - cached["timestamp"].max() > slack_ms
            ):
                raise ValueError("Requested time range isn't covered in cache file")

        return cached[
            (cached["timestamp"] >= start_ms) & (cached["timestamp"] <= end_ms)
        ].copy()

    def write_ohlvc(self, data: pd.DataFrame, source: SOURCE, interval: str) -> None:
        if interval not in self.DEFAULT_OHLCV_PERIOD_BY_INTERVAL:
            raise ValueError(f"Interval {interval} not supported by DataSource")

        data = self._clean_ohlvc(data)
        source_name = _normalize_source(source)

        for ticker in data["ticker"].unique():
            # Check path and file parent exist, create parent if not
            path: Path = self._cache_path(ticker, source_name, "ohlvc", interval)
            path.parent.mkdir(parents=True, exist_ok=True)

            ticker_data: pd.DataFrame = data[data["ticker"] == ticker].copy()
            if path.is_file():
                cached = self._clean_ohlvc(pd.read_parquet(path))
                cached = cached[cached["ticker"] == ticker]
                if not cached.empty:
                    ticker_data = (
                        pd.concat([cached, ticker_data], ignore_index=True)
                        .drop_duplicates(["timestamp", "ticker"], keep="last")
                        .sort_values(["timestamp", "ticker"])
                        .reset_index(drop=True)
                    )
            ticker_data.to_parquet(path, index=False)

    def coverage_ohlcv(
        self,
        *,
        tickers: Iterable[str],
        start: str | pd.Timestamp,
        end: str | pd.Timestamp,
        interval: str = "1d",
        source: str | SOURCE = SOURCE.YAHOO,
    ) -> list[Coverage]:
        # Checks and normalization
        if interval not in self.DEFAULT_OHLCV_PERIOD_BY_INTERVAL:
            raise ValueError(f"unsupported interval: {interval}")

        symbols = [str(t).strip().upper() for t in tickers]
        symbols = [t for t in symbols if t]
        if not symbols:
            raise ValueError("tickers must be non-empty")

        source_name = _normalize_source(source)
        req_start_ms = self._timestamp_to_ms(start)
        req_end_ms = self._timestamp_to_ms(end)

        covs: list[Coverage] = []

        for ticker in symbols:
            path = self._cache_path(ticker, source_name, "ohlvc", interval)
            rows = 0
            cache_start_ms = None
            cache_end_ms = None
            missing = [MissingRange(req_start_ms, req_end_ms)]

            if path.is_file() and path.stat().st_size > 0:
                try:
                    cached = self._clean_ohlvc(pd.read_parquet(path))
                    cached = cached[cached["ticker"] == ticker]
                except Exception:
                    cached = pd.DataFrame()

                if not cached.empty:
                    rows = len(cached)
                    cache_start_ms = int(cached["timestamp"].min())
                    cache_end_ms = int(cached["timestamp"].max())
                    missing = []
                    slack_ms = _coverage_slack_ms(interval)

                    if (
                        cache_end_ms + slack_ms < req_start_ms
                        or cache_start_ms - slack_ms > req_end_ms
                    ):
                        missing.append(MissingRange(req_start_ms, req_end_ms))
                    else:
                        if cache_start_ms - req_start_ms > slack_ms:
                            missing.append(MissingRange(req_start_ms, cache_start_ms))
                        if req_end_ms - cache_end_ms > slack_ms:
                            missing.append(MissingRange(cache_end_ms, req_end_ms))

            covs.append(
                Coverage(
                    ticker=ticker,
                    source=source_name,
                    start_ms=cache_start_ms,
                    end_ms=cache_end_ms,
                    rows=rows,
                    missing_ranges=tuple(missing),
                )
            )

        return covs

    def ensure_ohlcv(
        self,
        *,
        tickers: Iterable[str],
        start: str | pd.Timestamp,
        end: str | pd.Timestamp,
        interval: str = "1d",
        source: str | SOURCE = SOURCE.YAHOO,
        auto_adjust: bool = False,
    ) -> None:
        source_name = _normalize_source(source)
        coverages = self.coverage_ohlcv(
            tickers=tickers, start=start, end=end, interval=interval, source=source_name
        )
        fresh_list: List[pd.DataFrame] = []
        for cov in coverages:
            for mrange in cov.missing_ranges:
                s, e = _ms_to_timestamp(mrange.start_ms), _ms_to_timestamp(
                    mrange.end_ms
                )
                fresh = self.fetch_ohlvc_raw(
                    tickers=[cov.ticker],
                    start=s,
                    end=e,
                    interval=interval,
                    source=source_name,
                )
                fresh_list.append(self._clean_ohlvc(fresh))
        if not fresh_list:
            return
        data = pd.concat(fresh_list)
        self.write_ohlvc(data=data, source=source_name, interval=interval)

    def get_ohlvc(
        self,
        *,
        tickers: Iterable[str],
        start: str | pd.Timestamp,
        end: str | pd.Timestamp,
        interval: str = "1d",
        source: str | SOURCE = SOURCE.YAHOO,
        auto_adjust: bool = False,
        use_cache: Literal["cache_only", "refresh_missing", "bypass"] = "cache_only",
    ) -> pd.DataFrame:

        if interval not in self.DEFAULT_OHLCV_PERIOD_BY_INTERVAL:
            raise ValueError(f"Interval {interval} not supported by DataSource")
        symbols = [str(ticker).strip().upper() for ticker in tickers]
        symbols = [ticker for ticker in symbols if ticker]
        if not symbols:
            raise ValueError("tickers must be a non-empty iterable of symbols")
        valid_policies = {"cache_only", "refresh_missing", "bypass"}
        if use_cache not in valid_policies:
            raise ValueError(f"Unknown cache policy: {use_cache}")

        if use_cache == "bypass":
            data = self.fetch_ohlvc_raw(
                tickers=symbols,
                start=start,
                end=end,
                interval=interval,
                source=source,
                auto_adjust=auto_adjust,
            )
            return (
                self._clean_ohlvc(data)
                .sort_values(["timestamp", "ticker"])
                .reset_index(drop=True)
            )

        if use_cache == "refresh_missing":
            self.ensure_ohlcv(
                tickers=symbols,
                start=start,
                end=end,
                interval=interval,
                source=source,
                auto_adjust=auto_adjust,
            )

        data = []
        for symbol in symbols:
            cached = self.read_ohlvc(
                ticker=symbol,
                source=source,
                interval=interval,
                start=start,
                end=end,
                require_coverage=True,
            )
            if cached is None:
                raise ValueError(f"No cached OHLCV data for {symbol}")
            data.append(cached)
        return (
            pd.concat(data).sort_values(["timestamp", "ticker"]).reset_index(drop=True)
        )

    def to_panel(self, df: pd.DataFrame) -> pd.DataFrame:
        """
        Convert canonical flat OHLCV data into a (timestamp, ticker) MultiIndex panel.
        """
        self._validate_canonical_ohlcv(df)
        panel = df.copy().set_index(["timestamp", "ticker"]).sort_index()
        return panel

    def _cache_path(
        self,
        ticker: str,
        source: str | SOURCE,
        dataset_type: str,
        interval: str,
    ) -> Path:
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

        best_name = max(candidates, key=lambda name: valid_ratio(candidates[name]))
        best = candidates[best_name]
        if valid_ratio(best) >= 0.95:
            return best.round()
        return raw

    def _validate_ohlcv_loader_output(self, df: pd.DataFrame) -> None:
        if not isinstance(df, pd.DataFrame):
            raise TypeError("OHLCV loader must return a pandas.DataFrame")
        missing = [col for col in self.REQUIRED_OHLCV_COLUMNS if col not in df.columns]
        if missing:
            raise ValueError(f"OHLCV loader output missing required columns: {missing}")

    def _validate_canonical_ohlcv(self, df: pd.DataFrame) -> None:
        self._validate_ohlcv_loader_output(df)
        if df.empty:
            raise ValueError("Canonical OHLCV dataframe is empty")
        if df.duplicated(["timestamp", "ticker"]).any():
            raise ValueError(
                "Canonical OHLCV dataframe contains duplicate (timestamp, ticker) rows"
            )

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
            subset=["timestamp", "ticker", "open", "high", "low", "close", "volume"]
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


def _safe_path_part(value: object) -> str:
    text = str(value).strip().upper()
    text = re.sub(r"[^A-Z0-9._=-]+", "_", text)
    return text


def _normalize_source(source: str | SOURCE) -> str:
    if isinstance(source, SOURCE):
        return source.value
    return str(source).strip().lower()


def _ms_to_timestamp(ms: int) -> pd.Timestamp:
    return pd.to_datetime(ms, unit="ms", utc=True)


def _coverage_slack_ms(interval: str) -> int:
    if interval == "1d":
        return int(pd.Timedelta(days=7) // pd.Timedelta(milliseconds=1))
    if interval == "1h":
        return int(pd.Timedelta(days=3) // pd.Timedelta(milliseconds=1))
    if interval == "1m":
        return int(pd.Timedelta(days=1) // pd.Timedelta(milliseconds=1))
    return 0
