from typing import Dict
import pandas as pd
from ..portfolio import Position


def dollars_deltas(positions: Dict[str, Position], prices: Dict[str, float]) -> Dict[str, float]:
    return {t: positions[t].quantity_*prices[t] for t in positions}


def deltas(positions: Dict[str, Position]) -> Dict[str, float]:
    return {t: positions[t].quantity_ for t in positions}


def equity_returns(equity_ts: pd.DataFrame) -> pd.DataFrame:
    return equity_ts.pct_change()
