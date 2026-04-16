import numpy as np
import pandas as pd
import pytest

from ml.features.feature_engine import FeatureEngine, FeatureKind, FeatureSpec


def make_ohlcv(rows):
    return pd.DataFrame(
        rows,
        columns=["timestamp", "ticker", "open",
                 "high", "low", "close", "volume"],
    )


def test_feature_engine_builds_selected_feature_columns():
    df = make_ohlcv(
        [
            [1, "AAPL", 10, 12, 9, 10, 100],
            [2, "AAPL", 11, 13, 10, 11, 110],
            [3, "AAPL", 12, 14, 11, 12, 120],
        ]
    )
    engine = FeatureEngine()

    features = engine.build(
        df,
        specs=[
            FeatureSpec(FeatureKind.RETURN, (1,)),
            FeatureSpec(FeatureKind.SMA, (2,)),
            FeatureSpec(FeatureKind.HIGH_LOW_RANGE),
        ],
    )

    assert engine.feature_columns == ["ret_1d", "sma_2d", "high_low_range"]
    assert {"ret_1d", "sma_2d", "high_low_range"}.issubset(features.columns)
    assert features.loc[2, "sma_2d"] == 11.5
    assert features.loc[0, "high_low_range"] == pytest.approx(0.3)


def test_feature_engine_computes_features_per_ticker_without_cross_contamination():
    df = make_ohlcv(
        [
            [1, "AAPL", 100, 105, 95, 100, 1_000],
            [1, "MSFT", 200, 205, 195, 200, 2_000],
            [2, "AAPL", 110, 115, 105, 110, 1_100],
            [2, "MSFT", 220, 225, 215, 220, 2_200],
        ]
    )

    features = FeatureEngine().build(
        df,
        specs=[FeatureSpec(FeatureKind.RETURN, (1,))],
    )

    first_rows = features.groupby("ticker", sort=False).head(1)
    second_rows = features.groupby("ticker", sort=False).tail(1)

    assert first_rows["ret_1d"].isna().all()
    assert second_rows.set_index(
        "ticker").loc["AAPL", "ret_1d"] == pytest.approx(0.1)
    assert second_rows.set_index(
        "ticker").loc["MSFT", "ret_1d"] == pytest.approx(0.1)


def test_feature_engine_dropna_features_drops_warmup_rows():
    df = make_ohlcv(
        [
            [1, "AAPL", 10, 12, 9, 10, 100],
            [2, "AAPL", 11, 13, 10, 11, 110],
            [3, "AAPL", 12, 14, 11, 12, 120],
        ]
    )

    features = FeatureEngine().build(
        df,
        specs=[FeatureSpec(FeatureKind.SMA, (2,))],
        dropna_features=True,
    )

    assert len(features) == 2
    assert list(features["timestamp"]) == [2, 3]
    assert not features["sma_2d"].isna().any()


def test_feature_engine_filters_invalid_price_rows():
    df = make_ohlcv(
        [
            [1, "AAPL", 10, 12, 9, 10, 100],
            [2, "AAPL", 0, 13, 10, 11, 110],
            [3, "AAPL", 12, 14, 11, 12, 120],
        ]
    )

    features = FeatureEngine().build(
        df,
        specs=[FeatureSpec(FeatureKind.HIGH_LOW_RANGE)],
    )

    assert list(features["timestamp"]) == [1, 3]


def test_feature_engine_rejects_missing_required_columns():
    df = pd.DataFrame(
        {
            "timestamp": [1],
            "ticker": ["AAPL"],
            "close": [10],
        }
    )

    with pytest.raises(ValueError, match="Missing required columns"):
        FeatureEngine().build(df)


def test_feature_engine_rejects_duplicate_timestamp_ticker_rows():
    df = make_ohlcv(
        [
            [1, "AAPL", 10, 12, 9, 10, 100],
            [1, "AAPL", 11, 13, 10, 11, 110],
        ]
    )

    with pytest.raises(ValueError, match="Duplicate"):
        FeatureEngine().build(
            df, specs=[FeatureSpec(FeatureKind.HIGH_LOW_RANGE)])


def test_feature_engine_rejects_invalid_windows():
    df = make_ohlcv([[1, "AAPL", 10, 12, 9, 10, 100]])

    with pytest.raises(ValueError, match="window must be > 0"):
        FeatureEngine().build(
            df, specs=[FeatureSpec(FeatureKind.RETURN, (0,))])


def test_feature_engine_computes_log_return():
    df = make_ohlcv(
        [
            [1, "AAPL", 10, 12, 9, 10, 100],
            [2, "AAPL", 11, 13, 10, 11, 110],
        ]
    )

    features = FeatureEngine().build(
        df,
        specs=[FeatureSpec(FeatureKind.LOG_RETURN, (1,))],
    )

    assert features.loc[1, "log_ret_1d"] == pytest.approx(np.log(1.1))
