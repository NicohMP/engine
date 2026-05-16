#pragma once

#include "portfolio.hpp"
#include "data/extract/tick.hpp"
#include "core/event_types.hpp"
#include "data/features/feature_types.hpp"
#include <string>
#include <optional>
#include <vector>
#include <algorithm>

class Strategy {
    public:
        virtual ~Strategy() = default;
        virtual void reset() = 0;
        virtual std::optional<std::vector<OrderEvent>> on_market_event(
            const MarketEvent& mevent,
            const Portfolio& portfolio,
            const FeatureView& features
        ) = 0 ;
        virtual std::optional<std::vector<OrderEvent>> on_time_slice(
            const std::vector<MarketEvent>& events,
            const Portfolio& portfolio,
            const std::vector<FeatureView>& features
        );

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
