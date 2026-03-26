#pragma once

#include "account_state.hpp"
#include "position.hpp"
#include "tick.hpp"
#include "trading_types.hpp"

// Strategy contract for the backtesting engine.
// - Called once per tick in chronological order.
// - Receives current market tick, current position, and account snapshot.
// - Must return Buy/Sell/Hold action for this step.
class Strategy {
    public:
        virtual ~Strategy() = default;
        virtual Action on_tick(
            const Tick& tick,
            const Position& position,
            const AccountState& account_state) = 0;
};
