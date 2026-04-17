from typing import Optional, Protocol

import numpy as np
from collections import deque

from .base import Strategy, _build_order_to_target_notional, _flat_asset


class BetaEstimator(Protocol):
    def estimate_beta(
        self,
        p1: np.ndarray,
        p2: np.ndarray,
        window: int,
        old1: Optional[float] = None,
        old2: Optional[float] = None,
    ) -> Optional[float]: ...


class FixedBetaEstimator:
    def __init__(self, beta: float = 1.0):
        if not np.isfinite(beta):
            raise ValueError(f"beta must be finite, got {beta}")
        self.beta = float(beta)

    def estimate_beta(
        self,
        p1: np.ndarray,
        p2: np.ndarray,
        window: int,
        old1: Optional[float] = None,
        old2: Optional[float] = None,
    ) -> Optional[float]:
        _ = (p1, p2)
        return self.beta


class PairsStrategy(Strategy):

    def __init__(
        self,
        first: str,
        second: str,
        notional: float,
        zscore_window: int = 30,
        correlation_window: int = 100,
        corr_thresh: float = 0.8,
        entry_z: float = 2.0,
        exit_z: float = 0.5,
        beta_estimator: Optional[BetaEstimator] = None,
    ):
        if zscore_window <= 1 or correlation_window <= 1:
            raise ValueError("zscore_window and correlation_window must be > 1")
        if zscore_window > correlation_window:
            raise ValueError("zscore_window must be <= correlation_window")
        if notional <= 0.0:
            raise ValueError("notional must be > 0")
        self.first = first
        self.second = second
        self.notional = notional
        self.zwindow = zscore_window
        self.cwindow = correlation_window
        self._corr_thresh = corr_thresh
        self._entry_z = entry_z
        self.exit_z = exit_z
        self.beta_estimator = (
            beta_estimator if beta_estimator is not None else FixedBetaEstimator(1.0)
        )

        self.rolling_window = {
            self.first: deque(maxlen=self.cwindow),
            self.second: deque(maxlen=self.cwindow),
        }
        self._first_is_updated: bool = False
        self._second_is_updated: bool = False

    def compute_zscore(
        self,
        p1: np.ndarray,
        p2: np.ndarray,
        old1: Optional[float] = None,
        old2: Optional[float] = None,
    ) -> Optional[tuple[float, float, float]]:
        # simple returns on window (length window-1)
        r1 = p1[1:] / p1[:-1] - 1.0
        r2 = p2[1:] / p2[:-1] - 1.0

        # rolling-window correlation of returns
        corr = np.corrcoef(r1, r2)[0, 1]
        if not np.isfinite(corr):
            return None
        beta = self.beta_estimator.estimate_beta(p1, p2, self.cwindow, old1, old2)
        if beta is None or not np.isfinite(beta):
            return None

        # spread (log-price spread, beta=1 baseline)
        p1s = p1[-self.zwindow :]
        p2s = p2[-self.zwindow :]
        spread = np.log(p1s) - beta * np.log(p2s)

        # z-score of latest spread
        mu = spread.mean()
        sd = spread.std(ddof=0)
        if sd == 0.0 or np.isnan(sd):
            return None

        z = (spread[-1] - mu) / sd
        return z, corr, float(beta)

    def on_market_event(self, mevent, portfolio):
        old1: Optional[float] = None
        old2: Optional[float] = None
        # Fill/Update rolling window queue
        if mevent.ticker == self.first:
            if len(self.rolling_window[self.first]) == self.cwindow:
                old1 = self.rolling_window[self.first].popleft()
            self.rolling_window[self.first].append(mevent.close)
            self._first_is_updated = True
        elif mevent.ticker == self.second:
            if len(self.rolling_window[self.second]) == self.cwindow:
                old2 = self.rolling_window[self.second].popleft()
            self.rolling_window[self.second].append(mevent.close)
            self._second_is_updated = True
        else:
            return None

        # While long window isn't full, wait
        if (
            min(
                len(self.rolling_window[self.first]),
                len(self.rolling_window[self.second]),
            )
            < self.cwindow
        ):
            return None

        # Order if both price have been "recently" updated
        if self._first_is_updated and self._second_is_updated:
            # deque -> numpy
            p1 = np.asarray(self.rolling_window[self.first], dtype=float)
            p2 = np.asarray(self.rolling_window[self.second], dtype=float)
            out = self.compute_zscore(p1, p2, old1, old2)
            if out is None:
                self._first_is_updated = False
                self._second_is_updated = False
                return None
            z, corr, beta = out
            if corr < self._corr_thresh:
                self._first_is_updated = False
                self._second_is_updated = False
                return None

            # Get current quantity of each asset to build potential orders
            current_qty1 = portfolio.position_qty(self.first)
            current_qty2 = portfolio.position_qty(self.second)

            # When prices return within historical neighbourhood, flat both legs
            if abs(z) < self.exit_z:
                order1 = _flat_asset(ticker=self.first, current_quantity=current_qty1)
                order2 = _flat_asset(ticker=self.second, current_quantity=current_qty2)
                self._first_is_updated = False
                self._second_is_updated = False
                orders = [o for o in (order1, order2) if o is not None]
                return orders or None

            # If exit_z < |z| < entry_z, hold
            if abs(z) < self._entry_z:
                self._first_is_updated = False
                self._second_is_updated = False
                return None

            # Spread unusually high --> short spread
            if z > self._entry_z:
                signal1 = -1
                signal2 = +1
            # Spread unusually low --> long spread
            elif z < -self._entry_z:
                signal1 = +1
                signal2 = -1

            order1 = _build_order_to_target_notional(
                ticker=self.first,
                price=portfolio.price(self.first),
                signal=signal1,
                target_notional=self.notional,
                current_quantity=current_qty1,
            )
            order2 = _build_order_to_target_notional(
                ticker=self.second,
                price=portfolio.price(self.second),
                signal=signal2,
                target_notional=self.notional * abs(beta),
                current_quantity=current_qty2,
            )

            self._first_is_updated = False
            self._second_is_updated = False
            orders = [o for o in (order1, order2) if o is not None]
            return orders or None


class PairZscoreStrategy(PairsStrategy):
    """
    Backward-compatible alias around PairsStrategy (default fixed beta=1).
    """

    pass


class OLSBetaEstimator:
    def __init__(self):
        self.S1: Optional[float] = None
        self.S2: Optional[float] = None
        self.S12: Optional[float] = None
        self.S22: Optional[float] = None

    def estimate_beta(
        self,
        p1: np.ndarray,
        p2: np.ndarray,
        window: int,
        old1: Optional[float] = None,
        old2: Optional[float] = None,
    ) -> Optional[float]:
        if (
            (old1 is None)
            or (old2 is None)
            or (self.S1 is None)
            or (self.S2 is None)
            or (self.S12 is None)
            or (self.S22 is None)
        ):
            self.S1 = np.sum(p1)
            self.S2 = np.sum(p2)
            self.S12 = np.sum(p1 * p2)
            self.S22 = np.sum(p2 * p2)
        else:
            self.S1 += p1[-1] - old1
            self.S2 += p2[-1] - old2
            self.S12 += p2[-1] * p1[-1] - old2 * old1
            self.S22 += p2[-1] * p2[-1] - old2 * old2
        D = self.S22 - self.S2**2 / window
        if np.isclose(D, 0.0, atol=1e-12):
            return None

        beta = (self.S12 - self.S1 * self.S2 / window) / D
        return beta
