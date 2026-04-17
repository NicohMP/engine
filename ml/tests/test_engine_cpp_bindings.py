import sys
from pathlib import Path
import pytest

build_dir = Path(__file__).resolve().parents[2] / "build"
if build_dir.exists():
    sys.path.insert(0, str(build_dir))
engine_cpp = pytest.importorskip("engine_cpp")


class AlwaysBuy(engine_cpp.Strategy):
    def __init__(self):
        super().__init__()

    def on_market_event(self, mevent, portfolio):
        return [engine_cpp.OrderEvent(0, mevent.ticker, engine_cpp.TradeSide.Buy, 1.0)]


class RejectAll(engine_cpp.RiskManagerBase):
    def __init__(self):
        super().__init__()

    def accept_order_batch(self, orders, portfolio):
        return None


def test_python_strategy_and_risk_hooks_run_through_cpp_backtester():
    events = [
        engine_cpp.MarketEvent(1, "AAPL", 100.0, 101.0, 99.0, 1000.0, 100.0),
        engine_cpp.MarketEvent(2, "AAPL", 100.0, 102.0, 98.0, 1000.0, 101.0),
    ]
    bt = engine_cpp.Backtester(
        events, AlwaysBuy(), engine_cpp.Portfolio([], 1000.0), 0.0
    )
    bt.run()
    assert (len(bt.get_orders()), len(bt.get_fills())) == (2, 2)
    assert bt.get_equity() == [(1, 1000.0), (2, 1001.0)]
    blocked = engine_cpp.Backtester(
        [events[0]], AlwaysBuy(), engine_cpp.Portfolio([], 1000.0), RejectAll(), 0.0
    )
    blocked.run()
    assert blocked.get_orders() == []
    assert blocked.get_fills() == []
    assert blocked.get_equity() == [(1, 1000.0)]
