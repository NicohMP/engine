import pytest
import pandas as pd

from ml.data_ingestion.data_source import DataSource


def make_ohlvc(rows):
    return pd.DataFrame(
        rows,
        columns=["timestamp", "ticker", "open", "high", "low", "close", "volume"],
    )


def test_clean_ohlvc_keeps_one_row_per_timestamp_and_ticker():
    ds = DataSource(data_dir="/tmp/test-data-source")

    raw = make_ohlvc(
        [
            [1704067200000, "AAPL", 100, 110, 90, 105, 1_000_000],
            [1704067200000, "MSFT", 200, 210, 190, 205, 2_000_000],
        ]
    )

    clean = ds._clean_ohlvc(raw)

    assert len(clean) == 2
    assert set(clean["ticker"]) == {"AAPL", "MSFT"}
    assert not clean.duplicated(["timestamp", "ticker"]).any()


def test_clean_ohlvc_normalizes_tickers():
    ds = DataSource(data_dir="/tmp/test-data-source")

    raw = make_ohlvc(
        [[1704067200000, " aapl ", 100, 110, 90, 105, 1_000_000]]
    )

    clean = ds._clean_ohlvc(raw)

    assert clean.loc[0, "ticker"] == "AAPL"


def test_clean_ohlvc_normalizes_second_timestamps():
    ds = DataSource(data_dir="/tmp/test-data-source")

    raw = make_ohlvc(
        [[1704067200, "AAPL", 100, 110, 90, 105, 1_000_000]]
    )

    clean = ds._clean_ohlvc(raw)

    assert clean.loc[0, "timestamp"] == 1704067200000


def test_clean_ohlvc_rejects_missing_required_columns():
    ds = DataSource(data_dir="/tmp/test-data-source")

    raw = pd.DataFrame(
        {
            "timestamp": [1704067200000],
            "ticker": ["AAPL"],
            "open": [100],
            "high": [110],
            "low": [90],
            "close": [105],
        }
    )

    with pytest.raises(ValueError, match="missing required columns"):
        ds._clean_ohlvc(raw)


def test_clean_ohlvc_rejects_large_duplicate_collapse():
    ds = DataSource(data_dir="/tmp/test-data-source")

    raw = make_ohlvc(
        [[1704067200000, "AAPL", 100, 110, 90, 105, 1_000_000]]
        * 12
    )

    with pytest.raises(ValueError, match="duplicate"):
        ds._clean_ohlvc(raw)


def test_get_ohlvc_uses_registered_loader(tmp_path):
    ds = DataSource(data_dir=tmp_path)

    def fake_loader(
        *,
        tickers,
        start=None,
        end=None,
        period=None,
        interval="1d",
        auto_adjust=False,
    ):
        return pd.DataFrame(
            {
                "timestamp": [1704067200000],
                "ticker": ["AAPL"],
                "open": [100],
                "high": [110],
                "low": [90],
                "close": [105],
                "volume": [1_000_000],
            }
        )

    ds.register_ohlcv_loader("fake", fake_loader)

    df = ds.get_ohlvc(
        tickers=["AAPL"],
        start="2024-01-01",
        end="2024-01-01",
        source="fake",
        use_cache=False,
    )

    assert len(df) == 1
    assert df.loc[0, "ticker"] == "AAPL"
    assert df.loc[0, "close"] == 105


def test_get_ohlvc_period_request_bypasses_cache(tmp_path):
    ds = DataSource(data_dir=tmp_path)
    calls = []

    def fake_loader(
        *,
        tickers,
        start=None,
        end=None,
        period=None,
        interval="1d",
        auto_adjust=False,
    ):
        calls.append({"tickers": list(tickers), "period": period})
        return make_ohlvc(
            [[1704067200000, "AAPL", 100, 110, 90, 105, 1_000_000]]
        )

    ds.register_ohlcv_loader("fake", fake_loader)

    df = ds.get_ohlvc(
        tickers=["AAPL"],
        period="1y",
        source="fake",
        use_cache=True,
    )

    assert len(df) == 1
    assert calls == [{"tickers": ["AAPL"], "period": "1y"}]


def test_get_ohlvc_reads_fully_covered_cache_without_calling_loader(tmp_path):
    ds = DataSource(data_dir=tmp_path)
    cache_path = ds._cache_path("AAPL", "fake", "ohlvc", "1d")
    cache_path.parent.mkdir(parents=True)
    make_ohlvc(
        [
            [1704067200000, "AAPL", 100, 110, 90, 105, 1_000_000],
            [1704153600000, "AAPL", 105, 115, 95, 110, 1_100_000],
        ]
    ).to_parquet(cache_path, index=False)

    def failing_loader(**kwargs):
        raise AssertionError("Loader should not be called on full cache hit")

    ds.register_ohlcv_loader("fake", failing_loader)

    df = ds.get_ohlvc(
        tickers=["AAPL"],
        start="2024-01-01",
        end="2024-01-02",
        source="fake",
        use_cache=True,
    )

    assert len(df) == 2
    assert list(df["close"]) == [105, 110]


def test_get_ohlvc_fetches_only_cache_misses(tmp_path):
    ds = DataSource(data_dir=tmp_path)
    cache_path = ds._cache_path("AAPL", "fake", "ohlvc", "1d")
    cache_path.parent.mkdir(parents=True)
    make_ohlvc(
        [
            [1704067200000, "AAPL", 100, 110, 90, 105, 1_000_000],
            [1704153600000, "AAPL", 105, 115, 95, 110, 1_100_000],
        ]
    ).to_parquet(cache_path, index=False)
    calls = []

    def fake_loader(
        *,
        tickers,
        start=None,
        end=None,
        period=None,
        interval="1d",
        auto_adjust=False,
    ):
        calls.append(list(tickers))
        return make_ohlvc(
            [
                [1704067200000, "MSFT", 200, 210, 190, 205, 2_000_000],
                [1704153600000, "MSFT", 205, 215, 195, 210, 2_100_000],
            ]
        )

    ds.register_ohlcv_loader("fake", fake_loader)

    df = ds.get_ohlvc(
        tickers=["AAPL", "MSFT"],
        start="2024-01-01",
        end="2024-01-02",
        source="fake",
        use_cache=True,
    )

    assert calls == [["MSFT"]]
    assert len(df) == 4
    assert set(df["ticker"]) == {"AAPL", "MSFT"}


def test_update_ohlvc_writes_trimmed_cache_file(tmp_path):
    ds = DataSource(data_dir=tmp_path)
    ds.DEFAULT_OHLCV_ROWS = 2

    def fake_loader(
        *,
        tickers,
        start=None,
        end=None,
        period=None,
        interval="1d",
        auto_adjust=False,
    ):
        return make_ohlvc(
            [
                [1704067200000, "AAPL", 100, 110, 90, 105, 1_000_000],
                [1704153600000, "AAPL", 105, 115, 95, 110, 1_100_000],
                [1704240000000, "AAPL", 110, 120, 100, 115, 1_200_000],
            ]
        )

    ds.register_ohlcv_loader("fake", fake_loader)

    ds.update_ohlvc(["aapl"], source="fake", interval="1d")
    cached = pd.read_parquet(ds._cache_path("AAPL", "fake", "ohlvc", "1d"))

    assert len(cached) == 2
    assert list(cached["timestamp"]) == [1704153600000, 1704240000000]


def test_get_ohlvc_rejects_unknown_interval(tmp_path):
    ds = DataSource(data_dir=tmp_path)

    with pytest.raises(ValueError, match="not supported"):
        ds.get_ohlvc(
            tickers=["AAPL"],
            start="2024-01-01",
            end="2024-01-02",
            interval="7m",
            source="fake",
        )


def test_get_ohlvc_rejects_unknown_source(tmp_path):
    ds = DataSource(data_dir=tmp_path)

    with pytest.raises(ValueError, match="No OHLCV loader registered"):
        ds.get_ohlvc(
            tickers=["AAPL"],
            start="2024-01-01",
            end="2024-01-02",
            source="unknown",
            use_cache=False,
        )


def test_to_panel_returns_timestamp_ticker_multiindex(tmp_path):
    ds = DataSource(data_dir=tmp_path)
    clean = ds._clean_ohlvc(
        make_ohlvc(
            [
                [1704067200000, "AAPL", 100, 110, 90, 105, 1_000_000],
                [1704067200000, "MSFT", 200, 210, 190, 205, 2_000_000],
            ]
        )
    )

    panel = ds.to_panel(clean)

    assert panel.index.names == ["timestamp", "ticker"]
    assert panel.loc[(1704067200000, "AAPL"), "close"] == 105
