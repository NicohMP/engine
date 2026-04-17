from dataclasses import dataclass
from .events import TradeSide, MarketEvent, FillEvent
from typing import Dict, List


@dataclass
class Position:
    ticker_: str
    quantity_: float

    def __init__(self, ticker: str, init_quantity: float):
        self.ticker_ = ticker
        self.quantity_ = init_quantity

    def update(self, fill: FillEvent) -> None:
        if fill.ticker != self.ticker_:
            raise ValueError(
                f"Fill ticker {fill.ticker} doesn't match position ticker {self.ticker_}"
            )
        if fill.side == TradeSide.BUY:
            self.quantity_ += fill.quantity
        else:
            self.quantity_ -= fill.quantity


class Portfolio:
    positions_by_ticker: Dict[str, Position]
    cash: float
    prices: Dict[str, float]

    def __init__(self, init_positions: List[Position], init_cash: float):
        self.cash = init_cash
        self.positions_by_ticker = {p.ticker_: p for p in init_positions}
        self.prices = {}

    def update_prices(self, mevent: MarketEvent) -> None:
        self.prices[mevent.ticker] = mevent.close

    def update_position(self, fill: FillEvent) -> None:

        if fill.ticker not in self.positions_by_ticker:
            # If positions doesn't exist yet, deduce sign of quantity, price, and create new Position instance

            sign = 1 if (fill.side == TradeSide.BUY) else -1
            new_position = Position(
                ticker=fill.ticker, init_quantity=sign * fill.quantity
            )
            self.prices[fill.ticker] = fill.price
            self.positions_by_ticker[fill.ticker] = new_position
        else:
            # Otherwise use built-in Position.update()
            self.positions_by_ticker[fill.ticker].update(fill)

        # Update portfolio cash
        notional: float = fill.quantity * fill.price
        if fill.side == TradeSide.BUY:
            self.cash -= notional + fill.fees  # pay shares + fees
        else:
            self.cash += notional - fill.fees  # pay fees, get paid for sold shares

    def equity(self) -> float:
        """Compute total equity from updated prices and internal positions"""
        eq = float(self.cash)
        for ticker, pos in self.positions_by_ticker.items():
            if ticker not in self.prices:
                continue
            eq += float(pos.quantity_) * float(self.prices[ticker])
        return eq

    def position_qty(self, ticker: str) -> float:
        if ticker not in self.positions_by_ticker:
            raise KeyError(
                f"No position for ticker {ticker} in the portfolio currently"
            )
        return self.positions_by_ticker[ticker].quantity_

    def quantities(self) -> Dict[str, float]:
        return {
            t: self.positions_by_ticker[t].quantity_ for t in self.positions_by_ticker
        }

    def price(self, ticker: str) -> float:
        if ticker not in self.prices:
            raise KeyError(
                f"Prices for ticker {ticker} isn't currently followed in portfolio"
            )
        return self.prices[ticker]

    def weights(self) -> Dict[str, float]:
        return {
            t: self.positions_by_ticker[t].quantity_ * self.prices[t] / self.equity()
            for t in self.positions_by_ticker
        }

    def gross_exposure(self) -> float:
        return sum(
            [
                abs(self.positions_by_ticker[t].quantity_ * self.prices[t])
                for t in self.positions_by_ticker
            ]
        )

    def net_exposure(self) -> float:
        return sum(
            [
                self.positions_by_ticker[t].quantity_ * self.prices[t]
                for t in self.positions_by_ticker
            ]
        )
