from typing import List, Optional

import numpy as np

from ..events import MarketEvent, OrderEvent
from ..portfolio import Portfolio
from .base import Strategy, _build_order_on_notional


class RandomStrategy(Strategy):
    def __init__(
        self,
        universe: List[str],
        target_notional_per_ticker: float = 100.0,
        probabilities: Optional[List[float]] = None,
    ):
        if len(universe) == 0:
            raise ValueError("universe must be non-empty")
        if target_notional_per_ticker <= 0.0:
            raise ValueError(
                f"target_notional_per_ticker must be > 0, got {target_notional_per_ticker}"
            )

        if probabilities is None:
            probs = np.full(3, 1.0 / 3.0, dtype=float)
        else:
            probs = np.asarray(probabilities, dtype=float)
            if probs.shape != (3,):
                raise ValueError(
                    f"probabilities must have length 3, got shape {probs.shape}")
            if (probs < 0.0).any():
                raise ValueError("probabilities must be non-negative")
            if not np.isclose(probs.sum(), 1.0):
                raise ValueError(
                    f"probabilities must sum to 1.0, got {probs.sum()}")

        self.probs = probs
        self.universe = universe
        self.target_not = float(target_notional_per_ticker)
        self.rng = np.random.default_rng(42)

        super().__init__()

    def on_market_event(self, mevent: MarketEvent, portfolio: Portfolio) -> Optional[List[OrderEvent]]:
        if mevent.ticker not in self.universe:
            return None
        if mevent.close <= 0.0:
            raise ValueError(f"mevent.close must be > 0, got {mevent.close}")

        signal = int(self.rng.choice([-1, 0, 1], p=self.probs))
        if signal == 0:
            return None
        target_quantity = portfolio.position_qty(ticker=mevent.ticker)
        order = _build_order_on_notional(
            ticker=mevent.ticker, price=mevent.close, signal=signal, target_notional=self.target_not, current_quantity=target_quantity)
        if order is None:
            return None
        return [order]
