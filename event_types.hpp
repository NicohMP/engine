#pragma once
#include <cstdint>
#include <string>
#include <optional>
#include <variant>
#include <vector>
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
    std::optional<double> price; // signal price at which to fill the order or discard it 
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

std::uint64_t generate_order_id();
void reset_order_id_counter(std::uint64_t start_id = 1);

std::vector<MarketEvent> ticks_to_mevents(const std::vector<Tick>& ticks);
MarketEvent tick_to_mevent(const Tick& tick);