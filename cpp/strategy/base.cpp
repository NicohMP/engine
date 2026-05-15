#include "base.hpp"

#include <cmath>
#include <optional>
#include <stdexcept>
#include <vector>
#include <optional>
#include <string>


std::optional<OrderEvent> build_order_to_target_notional(
    const SymbolId& id, 
    double price, 
    double target_notional,
    double current_quantity
) {
    if (price <= 0.0){
        throw std::runtime_error("price must be > 0 for "+std::to_string(id)+" , got "+ std::to_string(price));
    }
    double target_qty = target_notional / price;
    double order_qty = target_qty - current_quantity;
    if (order_qty == 0.0){
        return std::nullopt;
    }
    TradeSide side = (order_qty > 0.0) ? TradeSide::Buy : TradeSide::Sell;
    OrderEvent order{generate_order_id(),id, side, std::abs(order_qty), OrderType::Market, std::nullopt};
    return order;
}

std::optional<OrderEvent> flat_asset(const SymbolId& id, double current_quantity){
    if (current_quantity==0.0){
        return std::nullopt;
    }
    TradeSide side = (current_quantity < 0.0) ? TradeSide::Buy : TradeSide::Sell;
    OrderEvent order{generate_order_id(),id, side, std::abs(current_quantity), OrderType::Market, std::nullopt};
    return order;
}

// Default time slice behaviour : apply evetn wise strategy sequentially, batch everything and return batch
std::optional<std::vector<OrderEvent>>  Strategy::on_time_slice(
    const std::vector<MarketEvent>& events,
    const Portfolio& portfolio,
    const std::vector<FeatureView>& features
) {
    // Check feature / event compatibility
    if (events.size() != features.size()) {
        throw std::runtime_error("events and features must have the same size");
    }
    // Orders batch container
    std::vector<OrderEvent> batch;
    // For each mevent, apply on_market_event and add new orders to the batch
    for (size_t i = 0; i < events.size(); ++i) {
        auto maybe_orders = on_market_event(events[i], portfolio, features[i]);

        if (maybe_orders) {
            batch.insert(batch.end(), maybe_orders->begin(), maybe_orders->end());
        }
    }
    // If no orders, return None like in on_market_event
    if (batch.empty()) {
        return std::nullopt;
    }
    // Return batched orders 
    return batch;
}