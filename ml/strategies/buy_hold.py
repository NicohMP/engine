import numpy as np
from typing import List, Optional, Dict
from .base import Strategy, _flat_asset
from ..events import OrderEvent, TradeSide, generate_order_id


class BuyHoldStrategy(Strategy):
    """Places orders once to reach equity distribution of `distribution`,
    then holds indefinitely

    Args:
        universe (List[str]): Trading universe
        Distribution (Dict[str, float]) : distribution of equity between assets in the universe. Must contain exactly universe as keys, and sum to 1.
    """

    def __init__(
        self,
        universe: List[str],
        distribution: Optional[Dict[str, float]] = None,
    ):
        self.universe = universe
        if distribution is not None:
            if (
                (len(distribution) != len(universe))
                or (not np.isclose(sum(distribution.values()), 1))
                or (set(distribution) != set(universe))
            ):
                raise ValueError(
                    "Distribution should have same number of elements as universe and sum to 1"
                )
            self.weights = distribution
        else:
            self.weights = {t: (1 / len(universe)) for t in universe}

        self.invested: bool = False

    def on_market_event(self, mevent, portfolio):
        if self.invested:
            return None

        if any(t not in portfolio.prices for t in self.universe):
            return None

        orders: List[OrderEvent] = []
        # For each asset considered, if price is stored in portfolio,
        # create order to reach target distribution
        for ticker in [t for t in self.universe if (t in portfolio.prices)]:
            target_not = self.weights[ticker] * portfolio.equity()
            current_not = 0
            if ticker in portfolio.positions_by_ticker:
                current_not = portfolio.position_qty(ticker) * portfolio.price(ticker)
            qty = (target_not - current_not) / portfolio.price(ticker)
            side = TradeSide.BUY if qty >= 0 else TradeSide.SELL
            order = OrderEvent(
                id=generate_order_id(),
                ticker=ticker,
                side=side,
                quantity=abs(qty),
            )
            orders.append(order)
        # If former positions on unwanted assets, flat them
        for ticker in portfolio.positions_by_ticker:
            if ticker not in self.universe:
                curr_qty = portfolio.position_qty(ticker)
                orders.append(_flat_asset(ticker=ticker, current_quantity=curr_qty))
        # All necessary investment have been made, no more orders to emit
        self.invested = True

        return orders
