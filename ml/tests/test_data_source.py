import pandas as pd
import pytest
from ml.data_ingestion.data_source import DataSource


def make_ohlvc(rows):
    return pd.DataFrame(
        rows, columns=["timestamp", "ticker", "open", "high", "low", "close", "volume"]
    )


AAPL = [
    [1704067200000, "AAPL", 100, 110, 90, 105, 1_000_000],
    [1704153600000, "AAPL", 105, 115, 95, 110, 1_100_000],
]
MSFT = [
    [1704067200000, "MSFT", 200, 210, 190, 205, 2_000_000],
    [1704153600000, "MSFT", 205, 215, 195, 210, 2_100_000],
]

def install_loader(ds, rows, calls):
    def loader(**kwargs):
        calls.append(list(kwargs["tickers"]))
        return make_ohlvc(rows)
    ds._ohlcv_loaders["fake"] = loader


def test_clean_ohlvc_normalizes_seconds_and_tickers():
    ds = DataSource(data_dir="/tmp/test-data-source")
    clean = ds._clean_ohlvc(
        make_ohlvc([[1704067200, " aapl ", 100, 110, 90, 105, 1_000_000]])
    )
    assert clean.loc[0, "timestamp"] == 1704067200000
    assert clean.loc[0, "ticker"] == "AAPL"


def test_get_ohlvc_bypass_fetches_loader_without_cache(tmp_path):
    ds = DataSource(data_dir=tmp_path)
    calls = []
    install_loader(ds, [[1704067200000, "aapl", 100, 110, 90, 105, 1_000_000]], calls)
    df = ds.get_ohlvc(
        tickers=["AAPL"],
        start="2024-01-01",
        end="2024-01-01",
        source="fake",
        use_cache="bypass",
    )
    assert calls == [["AAPL"]]
    assert df.loc[0, "ticker"] == "AAPL"
    assert df.loc[0, "close"] == 105


def test_get_ohlvc_cache_only_reads_covered_cache(tmp_path):
    ds = DataSource(data_dir=tmp_path)
    path = ds._cache_path("AAPL", "fake", "ohlvc", "1d")
    path.parent.mkdir(parents=True)
    make_ohlvc(AAPL).to_parquet(path, index=False)
    df = ds.get_ohlvc(
        tickers=["AAPL"],
        start="2024-01-01",
        end="2024-01-02",
        source="fake",
        use_cache="cache_only",
    )
    assert list(df["close"]) == [105, 110]


def test_get_ohlvc_refresh_missing_fetches_only_missing_tickers(tmp_path):
    ds = DataSource(data_dir=tmp_path)
    path = ds._cache_path("AAPL", "fake", "ohlvc", "1d")
    path.parent.mkdir(parents=True)
    make_ohlvc(AAPL).to_parquet(path, index=False)
    calls = []
    install_loader(ds, MSFT, calls)
    df = ds.get_ohlvc(
        tickers=["AAPL", "MSFT"],
        start="2024-01-01",
        end="2024-01-02",
        source="fake",
        use_cache="refresh_missing",
    )
    assert calls == [["MSFT"]]
    assert set(df["ticker"]) == {"AAPL", "MSFT"}


def test_get_ohlvc_rejects_invalid_cache_policy(tmp_path):
    ds = DataSource(data_dir=tmp_path)
    with pytest.raises(ValueError, match="Unknown cache policy"):
        ds.get_ohlvc(
            tickers=["AAPL"],
            start="2024-01-01",
            end="2024-01-02",
            source="fake",
            use_cache=False,
        )


def test_get_ohlvc_rejects_unknown_source_on_bypass(tmp_path):
    ds = DataSource(data_dir=tmp_path)
    with pytest.raises(ValueError, match="No OHLCV loader registered"):
        ds.get_ohlvc(
            tickers=["AAPL"],
            start="2024-01-01",
            end="2024-01-02",
            source="unknown",
            use_cache="bypass",
        )
