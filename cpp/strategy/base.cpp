#include "base.hpp"

#include <cmath>
#include <optional>
#include <stdexcept>
#include <string>


std::optional<OrderEvent> build_order_to_target_notional(
    const std::string& ticker, 
    double price, 
    double target_notional,
    double current_quantity
) {
    if (price <= 0.0){
        throw std::runtime_error("price must be > 0 for "+ticker+" , got "+ std::to_string(price));
    }
    double target_qty = target_notional / price;
    double order_qty = target_qty - current_quantity;
    if (order_qty == 0.0){
        return std::nullopt;
    }
    TradeSide side = (order_qty > 0.0) ? TradeSide::Buy : TradeSide::Sell;
    OrderEvent order{generate_order_id(),ticker, side, std::abs(order_qty), OrderType::Market, std::nullopt};
    return order;
}

std::optional<OrderEvent> flat_asset(const std::string& ticker, double current_quantity){
    if (current_quantity==0.0){
        return std::nullopt;
    }
    TradeSide side = (current_quantity < 0.0) ? TradeSide::Buy : TradeSide::Sell;
    OrderEvent order{generate_order_id(),ticker, side, std::abs(current_quantity), OrderType::Market, std::nullopt};
    return order;
}

