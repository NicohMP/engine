from __future__ import annotations

from collections import deque
from dataclasses import dataclass
from typing import Deque, List, Optional, Protocol, Tuple

from .events import (
    FillEvent,
    MarketEvent,
    OrderEvent,
    Event,
    generate_order_id,
)
from .portfolio import Portfolio
from .execution_engine import ExecutionEngine
from .risk.risk_manager import RiskManager


class StrategyProtocol(Protocol):
    def on_market_event(self, mevent: MarketEvent, portfolio: Portfolio) -> Optional[List[OrderEvent]]:
        ...


@dataclass(frozen=True)
class BacktestState:
    orders: List[OrderEvent]
    fills: List[FillEvent]
    equity: List[Tuple[int, float]]


class Backtester:
    """
    Single-symbol event-driven backtester mirrored from C++.
    """

    def __init__(
        self,
        ticker: Optional[str],
        market_events: List[MarketEvent],
        strategy: StrategyProtocol,
        portfolio: Portfolio,
        fee_rate: float = 0.0010,
        risk_manager: Optional[RiskManager] = None,
    ) -> None:
        # Fee rate checks
        if fee_rate < 0.0:
            raise ValueError(f"fee_rate must be non-negative, got {fee_rate}")

        self.ticker_ = ticker if ticker else "ALL"
        self.strategy_ = strategy
        self.risk_manager_ = risk_manager
        self.portfolio = portfolio
        self.execution_engine_ = ExecutionEngine(fee_rate=fee_rate)
        self.fee_rate_ = float(fee_rate)

        # Check and initialize MarketEvents
        if ticker and ticker != "ALL":
            filtered = [m for m in market_events if m.ticker == ticker]
        else:
            filtered = list(market_events)
        self.mevents_: List[MarketEvent] = sorted(
            filtered, key=lambda m: m.timestamp)
        if len(self.mevents_) == 0:
            raise ValueError(f"no data for ticker {self.ticker_}")

        self.event_queue_: Deque[Event] = deque()
        self.orders_: List[OrderEvent] = []
        self.fills_: List[FillEvent] = []
        self.equity_: List[Tuple[int, float]] = []

    # Main run methods passes market events, calls strategy, execution engine, updates portfolio
    def run(self) -> BacktestState:
        for mevent in self.mevents_:
            # Queue that adds newly created Order/FillEvent and processes before next MarketEvent
            self.event_queue_.clear()
            self.event_queue_.append(mevent)

            while len(self.event_queue_) > 0:
                event = self.event_queue_.popleft()

                if isinstance(event, MarketEvent):
                    market_event = event
                    # update portfolio prices with new close price
                    self.portfolio.update_prices(market_event)

                    maybe_orders = self.strategy_.on_market_event(
                        market_event, self.portfolio)
                    if maybe_orders is not None:
                        accepted_orders = maybe_orders
                        if self.risk_manager_ is not None:
                            accepted_orders = self.risk_manager_.accept_order_batch(
                                maybe_orders, self.portfolio)
                        if accepted_orders is None:
                            continue
                        for maybe_order in accepted_orders:
                            if maybe_order is None:
                                continue
                            order = maybe_order
                            if order.id <= 0:
                                # Same C++ behavior: backtester can auto-assign id if missing/invalid.
                                order = OrderEvent(
                                    id=generate_order_id(),
                                    ticker=order.ticker,
                                    side=order.side,
                                    quantity=order.quantity,
                                    order_type=order.order_type,
                                    price=order.price,
                                )
                            if order.quantity <= 0.0:
                                raise ValueError("Invalid order quantity")
                            self.event_queue_.append(order)

                    # Attempt fills even if no new order was emitted this bar.
                    fills = self.execution_engine_.fill_orders(market_event)
                    for fill in fills:
                        self.event_queue_.append(fill)
                    continue

                # Process eventual order
                if isinstance(event, OrderEvent):
                    order_event = event
                    self.orders_.append(order_event)
                    self.execution_engine_.pass_order(order_event)

                    fills = self.execution_engine_.fill_orders(mevent)
                    for fill in fills:
                        self.event_queue_.append(fill)
                    continue

                # Process eventual fill
                if isinstance(event, FillEvent):
                    fill_event = event
                    self.fills_.append(fill_event)
                    self.portfolio.update_position(fill_event)
                    continue

                raise TypeError(f"Unsupported event type: {type(event)}")

            # Update total equity
            self.equity_.append(
                (int(mevent.timestamp), self.portfolio.equity()))

        return BacktestState(
            orders=self.orders_.copy(),
            fills=self.fills_.copy(),
            equity=self.equity_.copy(),
        )

    def get_equity(self) -> List[Tuple[int, float]]:
        return self.equity_

    def get_orders(self) -> List[OrderEvent]:
        return self.orders_

    def get_fills(self) -> List[FillEvent]:
        return self.fills_

    def get_fee_rate(self) -> float:
        return self.fee_rate_
