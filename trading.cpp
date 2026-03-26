#include <string>
#include <stdexcept>
#include "position.hpp"
using std::string;

Position::Position(const string& ticker_, double init_quant) : ticker(ticker_), quantity(init_quant) {}

void Position::update(const Trade& trade) {
    if (trade.ticker != ticker) {
        throw std::runtime_error("Position ticker " +ticker+ " doesn't match trade ticker "+ trade.ticker);
    }
    if (trade.side == TradeSide::Buy) {
        quantity += trade.quantity;
    } else {
        quantity -= trade.quantity;
    }
}
