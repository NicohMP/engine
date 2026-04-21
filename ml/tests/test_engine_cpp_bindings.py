import sys
from pathlib import Path

import pytest

repo_root = Path(__file__).resolve().parents[2]
for candidate in (repo_root / "cpp" / "build", repo_root / "cpp" / "build-release", repo_root / "build"):
    if candidate.exists():
        sys.path.insert(0, str(candidate))
        break

engine_cpp = pytest.importorskip("engine_cpp")


class AlwaysBuy(engine_cpp.Strategy):
    def __init__(self):
        super().__init__()

    def on_market_event(self, mevent, portfolio):
        return [engine_cpp.OrderEvent(mevent.symbol_id, engine_cpp.TradeSide.Buy, 1.0)]


class RejectAll(engine_cpp.RiskManagerBase):
    def __init__(self):
        super().__init__()

    def accept_order_batch(self, orders, portfolio):
        return None


def make_store(rows):
    ticks = [engine_cpp.Tick(*row) for row in rows]
    return engine_cpp.MarketDataStore.from_ticks(ticks).store


def test_python_strategy_and_risk_hooks_run_through_cpp_backtester():
    data = make_store(
        [
            (1, "AAPL", 100.0, 101.0, 99.0, 1000.0, 100.0),
            (2, "AAPL", 100.0, 102.0, 98.0, 1000.0, 101.0),
        ]
    )
    bt = engine_cpp.Backtester(
        data, AlwaysBuy(), engine_cpp.Portfolio(1000.0), 0.0
    )
    bt.run()
    assert (len(bt.get_orders()), len(bt.get_fills())) == (2, 2)
    assert bt.get_equity() == [(1, 1000.0), (2, 1001.0)]

    blocked_data = make_store(
        [(1, "AAPL", 100.0, 101.0, 99.0, 1000.0, 100.0)]
    )
    blocked = engine_cpp.Backtester(
        blocked_data, AlwaysBuy(), engine_cpp.Portfolio(1000.0), RejectAll(), 0.0
    )
    blocked.run()
    assert blocked.get_orders() == []
    assert blocked.get_fills() == []
    assert blocked.get_equity() == [(1, 1000.0)]
