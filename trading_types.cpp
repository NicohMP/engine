#include "trading.hpp"


Action Action::sell(const std::string& ticker, double quantity) {
    if (quantity <= 0) {
        throw std::runtime_error("Can't sell negative or null amount of shares");
    }
    Action a;
    a.ticker = ticker;
    a.type = ActionType::Sell;
    a.quantity = quantity;
    a.order_type = OrderType::Market;

    return a;
}


Action Action::buy(const std::string& ticker, double quantity) {
    if (quantity <= 0) {
        throw std::runtime_error("Can't buy negative or null amount of shares");
    }
    Action a;
    a.ticker = ticker;
    a.type = ActionType::Buy;
    a.quantity = quantity;
    a.order_type = OrderType::Market;

    return a;
}


Action Action::hold(const std::string& ticker) {
  
    Action a;
    a.ticker = ticker;
    a.type = ActionType::Hold;
    a.quantity = std::nullopt;
    a.order_type = OrderType::Market;
    return a;
}


