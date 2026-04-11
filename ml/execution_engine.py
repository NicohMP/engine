from .events import OrderEvent,MarketEvent, FillEvent, OrderType, TradeSide
from typing import List

class ExecutionEngine:
    """
    Stage-2 execution model mirrored from C++:
    - Market orders fill immediately on current bar close.
    - Limit/Stop trigger on bar range (high/low), then fill on current bar close.
    - Orders are queued until filled (or forever if trigger never occurs).
    """

    def __init__(self, fee_rate: float = 0.0010) -> None:
        if fee_rate < 0.0:
            raise ValueError(f"fee_rate must be non-negative, got {fee_rate}")
        self.fee_rate = float(fee_rate)
        self.pending_orders: List[OrderEvent] = []

    def pass_order(self, order: OrderEvent) -> None:
        self.pending_orders.append(order)

    def _make_fill(self, order: OrderEvent, mevent: MarketEvent) -> FillEvent:
        if order.quantity <= 0.0:
            raise ValueError(f"Invalid order quantity: {order.quantity}")
        fill_price = float(mevent.close)
        notional = float(order.quantity) * fill_price
        fees = notional * self.fee_rate
        return FillEvent(
            order_id=order.id,
            timestamp=int(mevent.timestamp),
            ticker=order.ticker,
            side=order.side,
            quantity=float(order.quantity),
            price=fill_price,
            fees=fees,
        )

    def fill_orders(self, mevent: MarketEvent) -> List[FillEvent]:
        fills: List[FillEvent] = []
        still_pending: List[OrderEvent] = []

        for order in self.pending_orders:
            if order.ticker != mevent.ticker:
                still_pending.append(order)
                continue

            if order.order_type == OrderType.Market:
                fills.append(self._make_fill(order, mevent))
                continue

            if order.order_type == OrderType.Limit:
                if order.price is None:
                    raise ValueError("Limit order requires price")
                limit_hit = (
                    (order.side == TradeSide.BUY and mevent.low <= order.price)
                    or (order.side == TradeSide.SELL and mevent.high >= order.price)
                )
                if limit_hit:
                    fills.append(self._make_fill(order, mevent))
                else:
                    still_pending.append(order)
                continue

            if order.order_type == OrderType.Stop:
                if order.price is None:
                    raise ValueError("Stop order requires trigger price")
                stop_triggered = (
                    (order.side == TradeSide.BUY and mevent.high >= order.price)
                    or (order.side == TradeSide.SELL and mevent.low <= order.price)
                )
                if stop_triggered:
                    fills.append(self._make_fill(order, mevent))
                else:
                    still_pending.append(order)
                continue

            raise ValueError(f"Unsupported order type: {order.order_type}")

        self.pending_orders = still_pending
        return fills
