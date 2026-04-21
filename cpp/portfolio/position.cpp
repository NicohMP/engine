#include <string>
#include <stdexcept>
#include "position.hpp"
#include "event_types.hpp"
using std::string;

Position::Position(const SymbolId& id, double init_quant) : symbol_id(id), quantity(init_quant) {}

void Position::update(const FillEvent& fevent) {
    if (fevent.symbol_id != symbol_id) {
        throw std::runtime_error("Position id " +std::to_string(symbol_id)+ " doesn't match trade ticker "+ std::to_string(fevent.symbol_id));
    }
    if (fevent.side == TradeSide::Buy) {
        quantity += fevent.quantity;
    } else {
        quantity -= fevent.quantity;
    }
}
