#pragma once

#include "portfolio.hpp"
#include "tick.hpp"
#include "event_types.hpp"
#include <optional>

class Strategy {
    public:
        virtual ~Strategy() = default;
        virtual std::optional<OrderEvent> on_market_event(
            const MarketEvent& mevent,
            const Portfolio& portfolio) = 0 ;
};
