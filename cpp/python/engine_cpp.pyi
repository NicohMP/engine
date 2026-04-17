from __future__ import annotations

from collections.abc import Sequence
from enum import Enum
from typing import overload


class TradeSide(Enum):
    Buy: TradeSide
    Sell: TradeSide


class OrderType(Enum):
    Market: OrderType
    Limit: OrderType
    Stop: OrderType


class MarketEvent:
    timestamp: int
    ticker: str
    open: float
    high: float
    low: float
    volume: float
    close: float
    def __init__(
        self,
        timestamp: int,
        ticker: str,
        open: float,
        high: float,
        low: float,
        volume: float,
        close: float,
    ) -> None: ...


class OrderEvent:
    id: int
    ticker: str
    side: TradeSide
    quantity: float | None
    order_type: OrderType
    price: float | None
    @overload
    def __init__(
        self,
        id: int,
        ticker: str,
        side: TradeSide,
        quantity: float | None,
        order_type: OrderType = OrderType.Market,
        price: float | None = None,
    ) -> None: ...
    @overload
    def __init__(
        self,
        ticker: str,
        side: TradeSide,
        quantity: float | None,
        order_type: OrderType = OrderType.Market,
        price: float | None = None,
        id: int = 0,
    ) -> None: ...


class FillEvent:
    order_id: int
    timestamp: int
    ticker: str
    side: TradeSide
    quantity: float
    price: float
    fees: float
    def __init__(
        self,
        order_id: int,
        timestamp: int,
        ticker: str,
        side: TradeSide,
        quantity: float,
        price: float,
        fees: float,
    ) -> None: ...


class Position:
    def __init__(self, ticker: str, initial_quantity: float = 0.0) -> None: ...
    def get_quantity(self) -> float: ...
    def get_ticker(self) -> str: ...
    def update(self, fill_event: FillEvent) -> None: ...


class Portfolio:
    @overload
    def __init__(self, init_positions: Sequence[Position], init_cash: float) -> None: ...
    @overload
    def __init__(self, init_cash: float) -> None: ...
    def get_cash(self) -> float: ...
    def has_position(self, ticker: str) -> bool: ...
    def has_price(self, ticker: str) -> bool: ...
    def position_qty(self, ticker: str) -> float: ...
    def total_equity(self) -> float: ...
    def update_position(self, fill_event: FillEvent) -> None: ...
    def update_prices(self, market_event: MarketEvent) -> None: ...
    def price(self, ticker: str) -> float: ...
    def weights(self) -> dict[str, float]: ...
    def gross_exposure(self) -> float: ...
    def net_exposure(self) -> float: ...


class ExecutionEngine:
    def __init__(self, fee_rate: float = 0.0010) -> None: ...
    def pass_order(self, order: OrderEvent) -> None: ...
    def fill_orders(self, market_event: MarketEvent) -> list[FillEvent]: ...


class Strategy:
    def __init__(self) -> None: ...
    def on_market_event(
        self, mevent: MarketEvent, portfolio: Portfolio
    ) -> list[OrderEvent] | None: ...


class RandomStrategy(Strategy):
    def __init__(
        self,
        universe: Sequence[str],
        target_notional: float,
        probabilities: Sequence[float] | None = None,
        seed: int = 42,
    ) -> None: ...


class RiskManagerBase:
    def __init__(self) -> None: ...
    def accept_order_batch(
        self, orders: Sequence[OrderEvent], portfolio: Portfolio
    ) -> list[OrderEvent] | None: ...


class RiskLimits:
    max_position_weight: float
    max_gross_exposure: float
    max_net_exposure: float
    max_nb_positions: float
    def __init__(
        self,
        max_position_weight: float = 1.0,
        max_gross_exposure: float = 1.0,
        max_net_exposure: float = 50.0,
        max_nb_positions: float = 1000.0,
    ) -> None: ...


class SimpleRiskManager(RiskManagerBase):
    def __init__(self, limits: RiskLimits = ...) -> None: ...
    def acceptable(self, portfolio: Portfolio) -> bool: ...


class Backtester:
    @overload
    def __init__(
        self,
        market_events: Sequence[MarketEvent],
        strategy: Strategy,
        portfolio: Portfolio,
        fee_rate: float = 0.0010,
    ) -> None: ...
    @overload
    def __init__(
        self,
        market_events: Sequence[MarketEvent],
        strategy: Strategy,
        portfolio: Portfolio,
        risk_manager: RiskManagerBase | None,
        fee_rate: float = 0.0010,
    ) -> None: ...
    def run(self) -> None: ...
    def get_equity(self) -> list[tuple[int, float]]: ...
    def get_orders(self) -> list[OrderEvent]: ...
    def get_fills(self) -> list[FillEvent]: ...
    def get_fee_rate(self) -> float: ...


def add(a: int, b: int) -> int: ...
def generate_order_id() -> int: ...
def reset_order_id_counter(start_id: int = 1) -> None: ...
