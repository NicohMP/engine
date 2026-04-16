from dataclasses import dataclass
from enum import StrEnum
from typing import Callable, Iterable, Sequence

import pandas as pd

from .analytics import (
    feature_close_to_sma,
    feature_high_low_range,
    feature_log_return,
    feature_return,
    feature_sma,
    feature_volatility,
    feature_volume_zscore,
)


class FeatureKind(StrEnum):
    RETURN = "return"
    LOG_RETURN = "log_return"
    SMA = "sma"
    CLOSE_TO_SMA = "close_to_sma"
    VOLATILITY = "volatility"
    VOLUME_ZSCORE = "volume_zscore"
    HIGH_LOW_RANGE = "high_low_range"


@dataclass(frozen=True)
class FeatureSpec:
    kind: FeatureKind
    windows: tuple[int, ...] = ()


FeatureComputeFn = Callable[[pd.DataFrame, int | None], pd.Series]


@dataclass(frozen=True)
class FeatureDefinition:
    compute: FeatureComputeFn
    name_template: str
    requires_window: bool = True

    def column_name(self, window: int | None) -> str:
        if not self.requires_window:
            return self.name_template
        return self.name_template.format(window=window)


DEFAULT_FEATURES = (
    FeatureSpec(FeatureKind.RETURN, (1, 5, 20)),
    FeatureSpec(FeatureKind.LOG_RETURN, (1, 5, 20)),
    FeatureSpec(FeatureKind.SMA, (10, 20, 50)),
    FeatureSpec(FeatureKind.CLOSE_TO_SMA, (10, 20, 50)),
    FeatureSpec(FeatureKind.VOLATILITY, (20, 60)),
    FeatureSpec(FeatureKind.VOLUME_ZSCORE, (20,)),
    FeatureSpec(FeatureKind.HIGH_LOW_RANGE),
)

REQUIRED_OHLCV_COLUMNS = {
    "timestamp",
    "ticker",
    "open",
    "high",
    "low",
    "volume",
    "close",
}

FEATURE_DEFINITIONS: dict[FeatureKind, FeatureDefinition] = {
    FeatureKind.RETURN: FeatureDefinition(feature_return, "ret_{window}d"),
    FeatureKind.LOG_RETURN: FeatureDefinition(feature_log_return, "log_ret_{window}d",),
    FeatureKind.SMA: FeatureDefinition(feature_sma, "sma_{window}d"),
    FeatureKind.CLOSE_TO_SMA: FeatureDefinition(feature_close_to_sma, "close_to_sma_{window}d",),
    FeatureKind.VOLATILITY: FeatureDefinition(feature_volatility, "vol_{window}d"),
    FeatureKind.VOLUME_ZSCORE: FeatureDefinition(feature_volume_zscore, "volume_z_{window}d",),
    FeatureKind.HIGH_LOW_RANGE: FeatureDefinition(feature_high_low_range, "high_low_range", requires_window=False,),

}


class FeatureEngine:
    def __init__(self) -> None:
        self.feature_columns: list[str] = []

    def build(
        self,
        ohlcv: pd.DataFrame,
        specs: Sequence[FeatureSpec] = DEFAULT_FEATURES,
        *,
        dropna_features: bool = False,
    ) -> pd.DataFrame:
        _validate_required_columns(ohlcv, REQUIRED_OHLCV_COLUMNS)

        data = ohlcv.copy()
        data["ticker"] = data["ticker"].astype(str).str.strip().str.upper()
        data = data.sort_values(["ticker", "timestamp"]).reset_index(drop=True)

        data = data[
            (data["open"] > 0.0)
            & (data["high"] > 0.0)
            & (data["low"] > 0.0)
            & (data["close"] > 0.0)
            & (data["volume"] >= 0.0)
        ].copy()

        feature_columns = _feature_columns_from_specs(specs)
        parts: list[pd.DataFrame] = []

        for ticker, group in data.groupby("ticker", sort=False):
            g = group.sort_values("timestamp").reset_index(drop=True).copy()
            g["ticker"] = ticker

            for spec in specs:
                definition = _get_feature_definition(spec.kind)
                for window in _windows_for_spec(spec, definition):
                    g[definition.column_name(
                        window)] = definition.compute(g, window)

            parts.append(g)

        if not parts:
            self.feature_columns = feature_columns
            return pd.DataFrame(columns=list(ohlcv.columns) + feature_columns)

        features = (
            pd.concat(parts, ignore_index=True)
            .sort_values(["timestamp", "ticker"])
            .reset_index(drop=True)
        )

        if features.duplicated(["timestamp", "ticker"]).any():
            raise ValueError(
                "Duplicate (timestamp, ticker) rows found after feature build.")

        if dropna_features:
            features = features.dropna(
                subset=feature_columns).reset_index(drop=True)

        self.feature_columns = feature_columns
        return features


def _validate_required_columns(df: pd.DataFrame, required: Iterable[str]) -> None:
    missing = set(required) - set(df.columns)
    if missing:
        raise ValueError(f"Missing required columns: {sorted(missing)}")


def _get_feature_definition(kind: FeatureKind) -> FeatureDefinition:
    try:
        return FEATURE_DEFINITIONS[kind]
    except KeyError as exc:
        raise ValueError(f"Unsupported feature kind: {kind}") from exc


def _windows_for_spec(
    spec: FeatureSpec,
    definition: FeatureDefinition,
) -> tuple[int | None, ...]:
    if not definition.requires_window:
        if spec.windows:
            raise ValueError(f"{spec.kind} does not accept windows")
        return (None,)

    if not spec.windows:
        raise ValueError(f"{spec.kind} requires at least one window")
    for window in spec.windows:
        if window <= 0:
            raise ValueError(f"{spec.kind} window must be > 0, got {window}")
    return spec.windows


def _feature_columns_from_specs(specs: Sequence[FeatureSpec]) -> list[str]:
    columns: list[str] = []
    for spec in specs:
        definition = _get_feature_definition(spec.kind)
        columns.extend(
            definition.column_name(window)
            for window in _windows_for_spec(spec, definition)
        )
    return columns
