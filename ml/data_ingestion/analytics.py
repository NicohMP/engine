from pathlib import Path
from typing import Iterable

import numpy as np
import pandas as pd


def _validate_window(window: int) -> None:
    if window <= 0:
        raise ValueError(f"window must be > 0, got {window}")


def _validate_horizon(horizon: int) -> None:
    if horizon <= 0:
        raise ValueError(f"horizon must be > 0, got {horizon}")


def returns(df: pd.DataFrame) -> pd.Series:
    """Computes returns. Assumes input data is ohlvc ticks for a single symbol, already sorted by timestamp."""
    return df["close"].pct_change()


def log_returns(df: pd.DataFrame) -> pd.Series:
    return np.log(df["close"]).diff()


def cumulated_returns(df: pd.DataFrame, horizon: int = 5) -> pd.Series:
    _validate_horizon(horizon)
    if horizon >= len(df):
        raise ValueError(
            "horizon must be smaller than dataframe length in cumulated_returns")

    return df["close"].shift(-horizon) / df["close"] - 1


def cumulated_log_returns(df: pd.DataFrame, horizon: int = 5) -> pd.Series:
    _validate_horizon(horizon)
    if horizon >= len(df):
        raise ValueError(
            "horizon must be smaller than dataframe length in cumulated_log_returns")

    return np.log(df["close"].shift(-horizon) / df["close"])


def returns_to_ternary_labels(returns: pd.Series, thresh: float) -> pd.Series:
    """Takes a single ticker returns series (cumulative/log agnotstic) and turns into a ternary label"""
    if thresh < 0:
        raise ValueError(
            f"Threshold amplitude must be positive, got: {thresh}")
    labels = pd.Series(0, index=returns.index, dtype="int8")
    labels[returns > thresh] = 1
    labels[returns < -thresh] = -1
    labels[returns.isna()] = pd.NA
    return labels


def returns_to_binary_labels(returns: pd.Series) -> pd.Series:
    """Takes a single ticker returns series (cumulative/log agnotstic) and turns into a binary label"""
    labels = pd.Series(0, index=returns.index, dtype="int8")
    labels[returns > 0] = 1
    labels[returns.isna()] = pd.NA
    return labels


def past_returns(df: pd.DataFrame, horizon: int = 5) -> pd.Series:
    """Past simple return over horizon: close_t / close_{t-h} - 1."""
    _validate_horizon(horizon)
    return df["close"] / df["close"].shift(horizon) - 1.0


def rolling_volatility(ret: pd.Series, window: int = 20, annualize: bool = False, periods_per_year: int = 252) -> pd.Series:
    """Rolling std of returns. Uses population std (ddof=0)."""
    _validate_window(window)
    vol = ret.rolling(window=window, min_periods=window).std(ddof=0)
    if annualize:
        vol = vol * np.sqrt(periods_per_year)
    return vol


def rolling_realized_volatility_from_log_returns(
    log_ret: pd.Series,
    window: int = 20,
    annualize: bool = False,
    periods_per_year: int = 252,
) -> pd.Series:
    """
    Rolling realized volatility from log returns:
    RV_t = sqrt(sum_{i=t-window+1..t} r_i^2).
    """
    _validate_window(window)
    rv = np.sqrt(log_ret.pow(2).rolling(
        window=window, min_periods=window).sum())
    if annualize:
        # Convert window RV into annualized volatility on the same std scale.
        rv = rv * np.sqrt(periods_per_year / window)
    return rv


def rolling_mean_return(ret: pd.Series, window: int = 20, annualize: bool = False, periods_per_year: int = 252) -> pd.Series:
    _validate_window(window)
    mean_ret = ret.rolling(window=window, min_periods=window).mean()
    if annualize:
        mean_ret = mean_ret * periods_per_year
    return mean_ret


def rolling_sharpe(
    ret: pd.Series,
    window: int = 20,
    risk_free_rate_annual: float = 0.0,
    periods_per_year: int = 252,
    annualized: bool = True,
) -> pd.Series:
    """Rolling Sharpe using simple returns."""
    _validate_window(window)

    rf_per_period = risk_free_rate_annual / periods_per_year
    excess = ret - rf_per_period
    mu = excess.rolling(window=window, min_periods=window).mean()
    sigma = excess.rolling(window=window, min_periods=window).std(ddof=0)

    sharpe = mu / sigma.replace(0.0, np.nan)
    if annualized:
        sharpe = sharpe * np.sqrt(periods_per_year)
    return sharpe


def sma(df: pd.DataFrame, window: int = 20) -> pd.Series:
    _validate_window(window)
    return df["close"].rolling(window=window, min_periods=window).mean()


def price_vs_sma(df: pd.DataFrame, window: int = 20) -> pd.Series:
    ma = sma(df, window)
    return df["close"] / ma - 1.0


def sma_ratio(df: pd.DataFrame, fast: int = 5, slow: int = 20) -> pd.Series:
    _validate_window(fast)
    _validate_window(slow)
    if fast >= slow:
        raise ValueError(f"fast must be < slow, got fast={fast}, slow={slow}")
    return sma(df, fast) / sma(df, slow) - 1.0


def rolling_drawdown(df: pd.DataFrame, window: int = 20) -> pd.Series:
    """Drawdown from rolling peak over a fixed window."""
    _validate_window(window)
    peak = df["close"].rolling(window=window, min_periods=window).max()
    return df["close"] / peak - 1.0


def volume_zscore(df: pd.DataFrame, window: int = 20) -> pd.Series:
    _validate_window(window)
    v = df["volume"].astype(float)
    mu = v.rolling(window=window, min_periods=window).mean()
    sigma = v.rolling(window=window, min_periods=window).std(ddof=0)
    return (v - mu) / sigma.replace(0.0, np.nan)


FEATURE_COLUMNS = [
    "ret_1d",
    "logret_1d",
    "ret_5d",
    "ret_10d",
    "ret_20d",
    "vol_5d",
    "vol_20d",
    "sharpe_20d",
    "sma_ratio_5_20",
    "price_vs_sma20",
    "drawdown_20d",
    "volume_z_20d",
    "realized_vol_log_20d",
    "realized_vol_log_60d",
]

REQUIRED_OHLCV_COLUMNS = {"timestamp", "ticker",
                          "open", "high", "low", "volume", "close"}


def _validate_required_columns(df: pd.DataFrame, required: Iterable[str]) -> None:
    missing = set(required) - set(df.columns)
    if missing:
        raise ValueError(f"Missing required columns: {sorted(missing)}")


def _safe_corr(a: pd.Series, b: pd.Series, method: str) -> float:
    valid = pd.concat([a, b], axis=1).dropna()
    if valid.shape[0] < 2:
        return np.nan
    x = valid.iloc[:, 0]
    y = valid.iloc[:, 1]
    if x.nunique(dropna=True) <= 1:
        return np.nan
    if y.nunique(dropna=True) <= 1:
        return np.nan
    if method == "pearson":
        return x.corr(y, method="pearson")
    if method == "spearman":
        # Avoid hard dependency on scipy by using rank-transform + pearson.
        xr = x.rank(method="average")
        yr = y.rank(method="average")
        return xr.corr(yr, method="pearson")
    raise ValueError(f"Unsupported correlation method: {method}")


def build_feature_table(df: pd.DataFrame, horizon: int = 5) -> pd.DataFrame:
    """
    Build leakage-safe per-ticker features and future target.
    Target: y_logret_k = log(close[t+k] / close[t]).
    """
    _validate_horizon(horizon)
    _validate_required_columns(df, REQUIRED_OHLCV_COLUMNS)

    data = df.copy()
    data = data.sort_values(["ticker", "timestamp"]).reset_index(drop=True)

    # Basic quality guardrails before feature engineering.
    data = data[
        (data["open"] > 0.0)
        & (data["high"] > 0.0)
        & (data["low"] > 0.0)
        & (data["close"] > 0.0)
        & (data["volume"] >= 0.0)
    ].copy()

    parts = []
    for ticker, group in data.groupby("ticker", sort=False):
        g = group.sort_values("timestamp").reset_index(drop=True).copy()
        g["ticker"] = ticker

        # Momentum / return features
        g["ret_1d"] = returns(g)
        g["logret_1d"] = log_returns(g)
        g["ret_5d"] = past_returns(g, horizon=5)
        g["ret_10d"] = past_returns(g, horizon=10)
        g["ret_20d"] = past_returns(g, horizon=20)

        # Risk / quality features
        g["vol_5d"] = rolling_volatility(g["ret_1d"], window=5)
        g["vol_20d"] = rolling_volatility(g["ret_1d"], window=20)
        g["sharpe_20d"] = rolling_sharpe(
            g["ret_1d"], window=20, annualized=True)
        g["realized_vol_log_20d"] = rolling_realized_volatility_from_log_returns(
            g["logret_1d"], window=20
        )
        g["realized_vol_log_60d"] = rolling_realized_volatility_from_log_returns(
            g["logret_1d"], window=60
        )

        # Trend / mean-reversion features
        g["sma_ratio_5_20"] = sma_ratio(g, fast=5, slow=20)
        g["price_vs_sma20"] = price_vs_sma(g, window=20)

        # Stress / liquidity features
        g["drawdown_20d"] = rolling_drawdown(g, window=20)
        g["volume_z_20d"] = volume_zscore(g, window=20)

        # Future target
        g["y_logret_k"] = cumulated_log_returns(g, horizon=horizon)
        parts.append(g)

    feat = pd.concat(parts, ignore_index=True)
    feat = feat.dropna(subset=FEATURE_COLUMNS + ["y_logret_k"]).copy()

    # Data integrity checks
    if feat.duplicated(["timestamp", "ticker"]).any():
        raise ValueError(
            "Duplicate (timestamp, ticker) rows found after feature build.")

    return feat


def normalize_features_cross_sectional(
    feat: pd.DataFrame,
    feature_cols: list[str],
    by: str = "timestamp",
    method: str = "zscore",
    eps: float = 1e-12,
) -> pd.DataFrame:
    out = feat.copy()
    g = out.groupby(by)[feature_cols]

    if method == "zscore":
        mu = g.transform("mean")
        sd = g.transform(lambda s: s.std(ddof=0))
        out[feature_cols] = (out[feature_cols] - mu) / (sd + eps)
    elif method == "rank":
        ranks = g.rank(method="average", pct=True)
        out[feature_cols] = ranks - 0.5
    else:
        raise ValueError(f"Unsupported method: {method}")

    out[feature_cols] = out[feature_cols].fillna(0.0)
    return out


def time_split(
    df: pd.DataFrame,
    train: float = 0.7,
    val: float = 0.15,
    test: float = 0.15,
    by: str = "timestamp",
) -> tuple[pd.DataFrame, pd.DataFrame, pd.DataFrame]:
    """Chronological split by unique values of a time column."""
    if by not in df.columns:
        raise ValueError(f"Split column '{by}' not found in dataframe.")
    if min(train, val, test) <= 0:
        raise ValueError("train/val/test proportions must be > 0.")
    if not np.isclose(train + val + test, 1.0):
        raise ValueError(
            f"train+val+test must sum to 1.0, got {train + val + test}")

    unique_times = np.sort(df[by].unique())
    if unique_times.size < 3:
        raise ValueError(
            "Not enough unique timestamps for train/val/test split.")

    n = unique_times.size
    train_end_idx = max(1, int(np.floor(train * n)))
    val_end_idx = max(train_end_idx + 1, int(np.floor((train + val) * n)))
    val_end_idx = min(val_end_idx, n - 1)

    train_cut = unique_times[train_end_idx - 1]
    val_cut = unique_times[val_end_idx - 1]

    train_df = df[df[by] <= train_cut].copy()
    val_df = df[(df[by] > train_cut) & (df[by] <= val_cut)].copy()
    test_df = df[df[by] > val_cut].copy()
    return train_df, val_df, test_df


def feature_target_correlation(
    df: pd.DataFrame,
    feature_cols: list[str],
    target_col: str,
    methods: tuple[str, ...] = ("pearson", "spearman"),
) -> pd.DataFrame:
    """Pooled feature-target correlation report."""
    _validate_required_columns(df, set(feature_cols) | {target_col})
    rows = []
    for feature in feature_cols:
        row = {"feature": feature}
        for method in methods:
            corr = _safe_corr(df[feature], df[target_col], method=method)
            row[f"{method}_corr"] = corr
            row[f"abs_{method}_corr"] = np.abs(
                corr) if pd.notna(corr) else np.nan
        rows.append(row)
    return pd.DataFrame(rows).sort_values("feature").reset_index(drop=True)


def feature_target_correlation_by_ticker(
    df: pd.DataFrame,
    feature_cols: list[str],
    target_col: str,
    methods: tuple[str, ...] = ("pearson", "spearman"),
) -> tuple[pd.DataFrame, pd.DataFrame]:
    """
    Per-ticker feature-target correlations + aggregate summary.
    Returns (detail_df, summary_df).
    """
    _validate_required_columns(df, set(feature_cols) | {target_col, "ticker"})

    pooled = feature_target_correlation(df, feature_cols, target_col, methods)
    pooled_map = {
        (row["feature"], method): row[f"{method}_corr"]
        for _, row in pooled.iterrows()
        for method in methods
    }

    detail_rows = []
    for ticker, group in df.groupby("ticker", sort=False):
        for feature in feature_cols:
            for method in methods:
                corr = _safe_corr(
                    group[feature], group[target_col], method=method)
                detail_rows.append(
                    {
                        "ticker": ticker,
                        "feature": feature,
                        "method": method,
                        "corr": corr,
                    }
                )

    detail_df = pd.DataFrame(detail_rows)

    summary_rows = []
    grouped = detail_df.groupby(["feature", "method"], sort=True)
    for (feature, method), sub in grouped:
        vals = sub["corr"].dropna()
        pooled_corr = pooled_map.get((feature, method), np.nan)
        median_corr = vals.median() if not vals.empty else np.nan
        q75 = vals.quantile(0.75) if not vals.empty else np.nan
        q25 = vals.quantile(0.25) if not vals.empty else np.nan
        iqr = q75 - q25 if pd.notna(q75) and pd.notna(q25) else np.nan

        if vals.empty or pd.isna(pooled_corr) or pooled_corr == 0:
            sign_consistency = np.nan
        else:
            sign_consistency = (np.sign(vals) == np.sign(pooled_corr)).mean()

        summary_rows.append(
            {
                "feature": feature,
                "method": method,
                "pooled_corr": pooled_corr,
                "median_corr": median_corr,
                "iqr_corr": iqr,
                "sign_consistency_ratio": sign_consistency,
                "n_tickers_used": int(vals.shape[0]),
            }
        )

    summary_df = pd.DataFrame(summary_rows).sort_values(
        ["method", "feature"]).reset_index(drop=True)
    return detail_df, summary_df


def feature_feature_correlation(
    df: pd.DataFrame,
    feature_cols: list[str],
    method: str = "spearman",
) -> pd.DataFrame:
    """Pooled feature-feature correlation matrix."""
    _validate_required_columns(df, feature_cols)
    return df[feature_cols].corr(method=method)


def prune_collinear_features(
    df: pd.DataFrame,
    feature_cols: list[str],
    target_col: str,
    corr_method: str = "spearman",
    collinear_threshold: float = 0.90,
) -> tuple[list[str], list[str], pd.DataFrame]:
    """
    Drop one feature from highly collinear pairs.
    Keeps the feature with stronger |feature-target correlation|.
    Tie-break: drop lexicographically later feature.
    """
    if not (0.0 < collinear_threshold < 1.0):
        raise ValueError(
            f"collinear_threshold must be in (0, 1), got {collinear_threshold}")

    corr_report = feature_target_correlation(
        df, feature_cols, target_col, methods=(corr_method,))
    target_corr_map = corr_report.set_index(
        "feature")[f"{corr_method}_corr"].to_dict()

    ff = feature_feature_correlation(df, feature_cols, method=corr_method)

    pairs = []
    for i, fa in enumerate(feature_cols):
        for fb in feature_cols[i + 1:]:
            pair_corr = ff.loc[fa, fb]
            if pd.isna(pair_corr):
                continue
            if np.abs(pair_corr) > collinear_threshold:
                pairs.append((fa, fb, pair_corr))

    # Deterministic order: strongest collinearity first, then lexical pair.
    pairs.sort(key=lambda x: (-np.abs(x[2]), x[0], x[1]))

    dropped: set[str] = set()
    decisions = []

    for fa, fb, pair_corr in pairs:
        if fa in dropped or fb in dropped:
            continue

        ta = target_corr_map.get(fa, np.nan)
        tb = target_corr_map.get(fb, np.nan)
        abs_ta = np.abs(ta) if pd.notna(ta) else np.nan
        abs_tb = np.abs(tb) if pd.notna(tb) else np.nan

        # Exclude non-comparable (e.g. constant/NaN corr) from pruning decisions.
        if pd.isna(abs_ta) or pd.isna(abs_tb):
            continue

        if abs_ta < abs_tb:
            to_drop = fa
        elif abs_tb < abs_ta:
            to_drop = fb
        else:
            # Deterministic tie-break.
            to_drop = max(fa, fb)

        dropped.add(to_drop)
        decisions.append(
            {
                "feature_a": fa,
                "feature_b": fb,
                "pair_corr": pair_corr,
                "target_corr_a": ta,
                "target_corr_b": tb,
                "dropped": to_drop,
            }
        )

    selected = [f for f in feature_cols if f not in dropped]
    dropped_list = sorted(dropped)
    decisions_df = pd.DataFrame(decisions)
    return selected, dropped_list, decisions_df


def run_feature_correlation_pipeline(
    df: pd.DataFrame,
    data_dir: str | Path = "ml/data",
    horizon: int = 5,
    split_by: str = "timestamp",
    corr_methods: tuple[str, ...] = ("pearson", "spearman"),
    pruning_corr_method: str = "spearman",
    collinear_threshold: float = 0.90,
) -> dict:
    """
    Build features, split chronologically, compute train-only correlation reports,
    run pruning, and save report artifacts.
    """
    feat_df = build_feature_table(df, horizon=horizon)
    train_df, val_df, test_df = time_split(feat_df, by=split_by)

    pooled_corr = feature_target_correlation(
        train_df, FEATURE_COLUMNS, "y_logret_k", methods=corr_methods
    )
    per_ticker_detail, per_ticker_summary = feature_target_correlation_by_ticker(
        train_df, FEATURE_COLUMNS, "y_logret_k", methods=corr_methods
    )
    ff_corr = feature_feature_correlation(
        train_df, FEATURE_COLUMNS, method=pruning_corr_method
    )
    selected_features, dropped_features, pruning_decisions = prune_collinear_features(
        train_df,
        FEATURE_COLUMNS,
        "y_logret_k",
        corr_method=pruning_corr_method,
        collinear_threshold=collinear_threshold,
    )

    out_dir = Path(data_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    pooled_corr.to_csv(out_dir / "feature_target_corr_train.csv", index=False)
    per_ticker_summary.to_csv(
        out_dir / "feature_target_corr_by_ticker_train.csv", index=False)
    ff_corr.to_csv(out_dir / "feature_feature_corr_train.csv", index=True)
    pruning_decisions.to_csv(
        out_dir / "feature_pruning_decisions_train.csv", index=False)

    return {
        "feature_table": feat_df,
        "train_df": train_df,
        "val_df": val_df,
        "test_df": test_df,
        "feature_cols": FEATURE_COLUMNS.copy(),
        "selected_features": selected_features,
        "dropped_features": dropped_features,
        "feature_target_corr": pooled_corr,
        "feature_target_corr_by_ticker_detail": per_ticker_detail,
        "feature_target_corr_by_ticker_summary": per_ticker_summary,
        "feature_feature_corr": ff_corr,
        "pruning_decisions": pruning_decisions,
    }
