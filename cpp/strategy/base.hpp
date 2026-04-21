#pragma once

#include "portfolio.hpp"
#include "tick.hpp"
#include "event_types.hpp"
#include <string>
#include <optional>
#include <vector>
#include <algorithm>

class Strategy {
    public:
        virtual ~Strategy() = default;
        virtual std::optional<std::vector<OrderEvent>> on_market_event(
            const MarketEvent& mevent,
            const Portfolio& portfolio) = 0 ;
};


std::optional<OrderEvent> build_order_to_target_notional(
    const SymbolId& id, 
    double price, 
    double target_notional,
    double current_quantity
);


std::optional<OrderEvent> flat_asset(const SymbolId& id, double current_quantity);

inline bool is_close(double a, double b, double rtol = 1e-5, double atol = 1e-8) {
    return std::abs(a - b) <= (atol + rtol * std::abs(b));
}
