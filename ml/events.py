from enum import StrEnum
from dataclasses import dataclass
from typing import Optional, Union
from itertools import count


class TradeSide(StrEnum):
    BUY = "BUY"
    SELL = "SELL"


class OrderType(StrEnum):
    Market = "MARKET"
    Limit = "LIMIT"
    Stop = "STOP"


@dataclass(frozen=True, slots=True)
class MarketEvent:
    timestamp: int
    ticker: str
    open: float
    high: float
    close: float
    volume: float
    low: float


@dataclass(frozen=True, slots=True)
class OrderEvent:
    id: int
    ticker: str
    side: TradeSide
    quantity: Optional[float]
    order_type: OrderType = OrderType.Market
    price: Optional[float] = None


@dataclass(frozen=True, slots=True)
class FillEvent:
    order_id: int
    timestamp: int
    ticker: str
    side: TradeSide
    quantity: float
    price: float
    fees: float


Event = Union[MarketEvent, OrderEvent, FillEvent]


_order_id_counter = count(1)


def generate_order_id() -> int:
    return next(_order_id_counter)


def reset_order_id_counter(start: int = 1) -> None:
    if start <= 0:
        raise ValueError(f"start must be > 0, got {start}")
    global _order_id_counter
    _order_id_counter = count(start)
