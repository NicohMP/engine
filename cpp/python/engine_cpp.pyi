from __future__ import annotations

from collections.abc import Sequence
from enum import Enum
from typing import overload

SymbolId = int


class TradeSide(Enum):
    Buy: TradeSide
    Sell: TradeSide


class OrderType(Enum):
    Market: OrderType
    Limit: OrderType
    Stop: OrderType


class Tick:
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


class TimeSlice:
    timestamp: int
    begin_idx: int
    end_idx: int


class BuildStats:
    input_rows: int
    kept_rows: int
    dropped_invalid: int
    dropped_duplicates: int
    num_symbols: int
    num_slices: int
    def __init__(self) -> None: ...


class MarketEvent:
    timestamp: int
    symbol_id: SymbolId
    open: float
    high: float
    low: float
    volume: float
    close: float
    def __init__(
        self,
        timestamp: int,
        symbol_id: SymbolId,
        open: float,
        high: float,
        low: float,
        volume: float,
        close: float,
    ) -> None: ...


class MarketDataStore:
    @staticmethod
    def from_ticks(rows: Sequence[Tick]) -> BuildResult: ...
    def get_or_add_id(self, ticker: str) -> SymbolId: ...
    def get_symbol(self, symbol_id: SymbolId) -> str: ...
    def get_id(self, ticker: str) -> SymbolId: ...
    def bars(self) -> list[MarketEvent]: ...
    def slices(self) -> list[TimeSlice]: ...


class BuildResult:
    store: MarketDataStore
    stats: BuildStats


class OrderEvent:
    id: int
    symbol_id: SymbolId
    side: TradeSide
    quantity: float | None
    order_type: OrderType
    price: float | None
    @overload
    def __init__(
        self,
        id: int,
        symbol_id: SymbolId,
        side: TradeSide,
        quantity: float | None,
        order_type: OrderType = OrderType.Market,
        price: float | None = None,
    ) -> None: ...
    @overload
    def __init__(
        self,
        symbol_id: SymbolId,
        side: TradeSide,
        quantity: float | None,
        order_type: OrderType = OrderType.Market,
        price: float | None = None,
        id: int = 0,
    ) -> None: ...


class FillEvent:
    order_id: int
    timestamp: int
    symbol_id: SymbolId
    side: TradeSide
    quantity: float
    price: float
    fees: float
    def __init__(
        self,
        order_id: int,
        timestamp: int,
        symbol_id: SymbolId,
        side: TradeSide,
        quantity: float,
        price: float,
        fees: float,
    ) -> None: ...


class Position:
    def __init__(self, symbol_id: SymbolId, initial_quantity: float = 0.0) -> None: ...
    def get_quantity(self) -> float: ...
    def get_id(self) -> SymbolId: ...
    def update(self, fill_event: FillEvent) -> None: ...


class Portfolio:
    @overload
    def __init__(self, init_positions: Sequence[Position], init_cash: float) -> None: ...
    @overload
    def __init__(self, init_cash: float) -> None: ...
    def get_cash(self) -> float: ...
    def has_position(self, symbol_id: SymbolId) -> bool: ...
    def has_price(self, symbol_id: SymbolId) -> bool: ...
    def position_qty(self, symbol_id: SymbolId) -> float: ...
    def total_equity(self) -> float: ...
    def update_position(self, fill_event: FillEvent) -> None: ...
    def update_prices(self, market_event: MarketEvent) -> None: ...
    def price(self, symbol_id: SymbolId) -> float: ...
    def weights(self) -> dict[SymbolId, float]: ...
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
        universe_ids: Sequence[SymbolId],
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
        data: MarketDataStore,
        strategy: Strategy,
        portfolio: Portfolio,
        fee_rate: float = 0.0010,
    ) -> None: ...
    @overload
    def __init__(
        self,
        data: MarketDataStore,
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
