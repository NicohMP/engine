from dataclasses import dataclass
from ml.portfolio import Portfolio, Position
from ml.events import OrderEvent, FillEvent
from typing import List, Optional


@dataclass(frozen=True)
class RiskLimits:
    max_position_weight: float = 0.1
    max_gross_exposure: float = 1.00
    max_net_exposure: float = 0.20
    max_nb_positions: int = 40


class RiskManager:
    def __init__(self, limits: RiskLimits):
        self.limits = limits

    def accept_order_batch(self, orders: List[OrderEvent], portfolio: Portfolio) -> Optional[List[OrderEvent]]:
        """Create a shadow portfolio (copy), then apply the orders to the shadow, and check if the resulting 
        exposure is acceptable. If so, pass the orders, otherwise return nothing. """
        shadow = copy_portfolio(portfolio=portfolio)
        for order in orders:
            apply_hypothetical_order(shadow, order)
        if not self.acceptable(shadow):
            return None
        return orders

    def acceptable(self, portfolio: Portfolio) -> bool:
        """Check hypothetical portfolio against chosen risk limits"""
        res = True
        gross = abs(portfolio.gross_exposure() / portfolio.equity())
        net = abs(portfolio.net_exposure() / portfolio.equity())
        if gross > self.limits.max_gross_exposure:
            res = False
        if net > self.limits.max_net_exposure:
            res = False
        # Count only asset with non-zero position
        if sum(1 for p in portfolio.positions_by_ticker.values() if p.quantity_ != 0) > self.limits.max_nb_positions:
            res = False
        # If weights dictionnary is empty (e.g before first order), default to maximum weight = 0.
        if max((abs(w) for w in portfolio.weights().values()), default=0.0) > self.limits.max_position_weight:
            res = False
        return res


def copy_portfolio(portfolio: Portfolio) -> Portfolio:
    """Copies portfolio class instance to test the hypothetical orders' effect on the exposure """
    copied_positions = [
        Position(pos.ticker_, pos.quantity_)
        for pos in portfolio.positions_by_ticker.values()
    ]

    copied = Portfolio(
        init_positions=copied_positions,
        init_cash=portfolio.cash,
    )

    copied.prices = portfolio.prices.copy()
    return copied


def apply_hypothetical_order(portfolio: Portfolio, order: OrderEvent) -> None:
    """Applies evaluated order on the shadow portfolio"""
    fill = FillEvent(
        order_id=order.id,
        timestamp=0,
        ticker=order.ticker,
        side=order.side,
        quantity=order.quantity,
        price=portfolio.price(order.ticker),
        fees=0.0,
    )
    if fill.ticker not in portfolio.positions_by_ticker:
        portfolio.positions_by_ticker[fill.ticker] = Position(
            ticker=fill.ticker, init_quantity=0.0)
    portfolio.update_position(fill)
