#pragma once
#include <cstdint>
#include <string>
#include <optional>
#include <variant>
#include <vector>
#include "tick.hpp"

enum class TradeSide { Buy, Sell };
enum class OrderType { Market, Limit, Stop };
using SymbolId = std::uint32_t;


struct MarketEvent {
    int64_t timestamp;
    SymbolId symbol_id;
    std::string ticker;
    double open;
    double high;
    double low;
    double volume;
    double close;
};

struct OrderEvent {
    std::uint64_t id;
    SymbolId symbol_id;
    std::string ticker;
    TradeSide side;
    std::optional<double> quantity;
    OrderType order_type = OrderType::Market;
    std::optional<double> price; // signal price at which to fill the order or discard it 
};

struct FillEvent {
    std::uint64_t order_id;
    int64_t timestamp;
    SymbolId symbol_id;
    std::string ticker;
    TradeSide side;
    double quantity;
    double price;
    double fees;
};

using Event = std::variant<MarketEvent, OrderEvent, FillEvent>;

std::uint64_t generate_order_id();
void reset_order_id_counter(std::uint64_t start_id = 1);

