#pragma once

#include "portfolio.hpp"
#include "tick.hpp"
#include "event_types.hpp"
#include <string>
#include <optional>
#include <vector>

class Strategy {
    public:
        virtual ~Strategy() = default;
        virtual std::optional<std::vector<OrderEvent>> on_market_event(
            const MarketEvent& mevent,
            const Portfolio& portfolio) = 0 ;
};


std::optional<OrderEvent> build_order_to_target_notional(
    const std::string& ticker, 
    double price, 
    double target_notional,
    double current_quantity
);


std::optional<OrderEvent> flat_asset(const std::string& ticker, double current_quantity);
