from typing import Optional, List
from ..portfolio import Portfolio
from ..events import MarketEvent, OrderEvent, OrderType, TradeSide, generate_order_id


class Strategy:
    def on_market_event(
        self, mevent: MarketEvent, portfolio: Portfolio
    ) -> Optional[List[OrderEvent]]:
        """To be overriden in children."""
        return None


def _build_order_on_notional(
    ticker: str,
    price: float,
    target_notional: float,
    signal: float,
    current_quantity: float,
) -> Optional[OrderEvent]:
    """_summary_

    Args:
        ticker (str): _description_
        price (float): _description_
        target_notional (float): _description_
        signal (float): _description_
        current_quantity (float): _description_

    Raises:
        ValueError: _description_

    Returns:
        Optional[OrderEvent]: _description_
    """
    if price <= 0.0:
        raise ValueError(f"price must be > 0 for {ticker}, got {price}")
    target_quantity: float = signal * target_notional / price
    order_quantity = target_quantity - current_quantity
    if order_quantity == 0.0:
        return None

    side = TradeSide.BUY if (order_quantity > 0.0) else TradeSide.SELL
    order = OrderEvent(
        id=generate_order_id(),
        ticker=ticker,
        side=side,
        quantity=abs(order_quantity),
        order_type=OrderType.Market,
    )
    if order.quantity <= 0.0:
        return None
    return order


def _flat_asset(ticker: str, current_quantity: float) -> Optional[OrderEvent]:
    """Create OrderEvent to flat the position on the ticker (i.e reduce the quantity of asset in portfolio to 0)

    Args:
        ticker (str): Position to flat
        current_quantity (float): Current quantity of asset of interest in the portfolio

    Returns:
        Optional[OrderEvent]: Maybe OrderEvent to flat the asset (returns None if position is already flat)
    """
    if current_quantity == 0:
        return None
    side = TradeSide.BUY if (current_quantity < 0) else TradeSide.SELL
    order = OrderEvent(
        id=generate_order_id(),
        ticker=ticker,
        side=side,
        quantity=abs(current_quantity),
        order_type=OrderType.Market,
    )
    if order.quantity <= 0.0:
        return None
    return order
