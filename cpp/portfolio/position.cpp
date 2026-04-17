#include <string>
#include <stdexcept>
#include "position.hpp"
#include "event_types.hpp"
using std::string;

Position::Position(const string& ticker_, double init_quant) : ticker(ticker_), quantity(init_quant) {}

void Position::update(const FillEvent& fevent) {
    if (fevent.ticker != ticker) {
        throw std::runtime_error("Position ticker " +ticker+ " doesn't match trade ticker "+ fevent.ticker);
    }
    if (fevent.side == TradeSide::Buy) {
        quantity += fevent.quantity;
    } else {
        quantity -= fevent.quantity;
    }
}
