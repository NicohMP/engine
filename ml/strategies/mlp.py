from typing import List, Optional, Dict

import pandas as pd
import torch
import torch.nn as nn
from collections import deque

from ..events import MarketEvent, OrderEvent
from ..portfolio import Portfolio
from ..features import analytics as an
from .base import Strategy, _build_order_on_notional


class DumbMLPStrategy(Strategy):
    """Pretrained MLP on historical data. The MLP just takes as

    Args:
        Strategy (_type_): _description_
    """

    def __init__(self, mlp: nn.Module, universe: List[str], target_notional: float):
        if len(universe) == 0:
            raise ValueError("universe must be non-empty")
        if target_notional <= 0.0:
            raise ValueError(
                f"target_notional must be > 0, got {target_notional}")

        self.model = mlp
        self.model.eval()
        self.universe = universe
        self.target_not = float(target_notional)
        self.windows: Dict[str, deque] = {
            t: deque(maxlen=30) for t in self.universe}

        super().__init__()

    def rolling_window_features(self, ticker: str) -> Optional[pd.Series]:
        "Builds the rolling window features for the MarketEvent features"
        if ticker not in self.windows:
            raise KeyError(
                f"Ticker : {ticker} is not followed by rolling windows")
        if len(self.windows[ticker]) < 30:
            return None

        g = pd.DataFrame(list(self.windows[ticker]))
        # Momentum / return features
        g["ret_1d"] = an.returns(g)
        g["logret_1d"] = an.log_returns(g)
        g["ret_5d"] = an.past_returns(g, horizon=5)
        g["ret_10d"] = an.past_returns(g, horizon=10)
        g["ret_20d"] = an.past_returns(g, horizon=20)

        # Risk / quality features
        g["vol_5d"] = an.rolling_volatility(g["ret_1d"], window=5)
        g["vol_20d"] = an.rolling_volatility(g["ret_1d"], window=20)
        g["sharpe_20d"] = an.rolling_sharpe(
            g["ret_1d"], window=20, annualized=True)

        # Trend / mean-reversion features
        g["sma_ratio_5_20"] = an.sma_ratio(g, fast=5, slow=20)
        g["price_vs_sma20"] = an.price_vs_sma(g, window=20)

        # Stress / liquidity features
        g["drawdown_20d"] = an.rolling_drawdown(g, window=20)
        g["volume_z_20d"] = an.volume_zscore(g, window=20)
        out = g[an.FEATURE_COLUMNS].iloc[-1]
        if out.isna().any():
            return None
        return out

    def on_market_event(self, mevent: MarketEvent, portfolio: Portfolio) -> Optional[List[OrderEvent]]:
        # Check if ticker is considered by the strategy
        if mevent.ticker not in self.universe:
            return None
        if mevent.close <= 0.0:
            raise ValueError(f"mevent.close must be > 0, got {mevent.close}")
        # Add tick to rolling windows
        self.windows[mevent.ticker].append({
            "timestamp": mevent.timestamp,
            "ticker": mevent.ticker,
            "open": mevent.open,
            "high": mevent.high,
            "low": mevent.low,
            "close": mevent.close,
            "volume": mevent.volume,
        })

        feat = self.rolling_window_features(mevent.ticker)
        if feat is None:
            return None

        device = next(self.model.parameters()).device
        feat_tensor = torch.tensor(
            feat.to_numpy(), dtype=torch.float32, device=device
        ).unsqueeze(0)

        with torch.no_grad():
            logits = self.model(feat_tensor)
            pred_class = int(torch.argmax(logits, dim=1).item())

        # Class convention: {0,1,2} -> {-1,0,1}
        signal = pred_class - 1
        if signal == 0:
            return None

        if mevent.ticker not in portfolio.positions_by_ticker:
            raise KeyError(
                f"Ticker {mevent.ticker} not found in portfolio positions")
        target_quantity = portfolio.positions_by_ticker[mevent.ticker].quantity_
        order = _build_order_on_notional(
            ticker=mevent.ticker, price=mevent.close, signal=signal, target_notional=self.target_not, current_quantity=target_quantity)
        if order is None:
            return None
        return [order]
