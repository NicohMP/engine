#pragma once

#include <cstdint>
#include <optional>
#include <string>

enum class TradeSide { Buy, Sell };

enum class ActionType { Buy, Sell, Hold };

enum class OrderType { Market, Limit };

struct Trade {
    int64_t timestamp;
    std::string ticker;
    TradeSide side;
    double quantity;
    double price;
};

struct Action {
    std::string ticker;
    ActionType type;
    std::optional<double> quantity;
    OrderType order_type = OrderType::Market;

    static Action sell(const std::string& ticker, double quantity);
    static Action buy(const std::string& ticker, double quantity);
    static Action hold(const std::string& ticker);
};
