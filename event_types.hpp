#pragma once
#include <cstdint>
#include <string>
#include <optional>
#include <cstdint>
#include <variant>
#include "trading_types.hpp"
#include "tick.hpp"


struct MarketEvent {
    Tick tick;
};

struct OrderEvent {
    std::uint64_t id;
    std::string ticker;
    TradeSide side;
    std::optional<double> quantity;
    OrderType order_type = OrderType::Market;
};

struct FillEvent {
    std::uint64_t order_id;
    int64_t timestamp;
    std::string ticker;
    TradeSide side;
    double quantity;
    double price;
    double fees;
};

using Event = std::variant<MarketEvent, OrderEvent, FillEvent>;


