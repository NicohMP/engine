#pragma once

#include <string>
#include "core/event_types.hpp"

class Position {
    private:
        SymbolId symbol_id;
        double quantity;

    public:
        Position(const SymbolId& id, double initial_quantity);
        double get_quantity() const { return quantity; }
        SymbolId get_id() const { return symbol_id; }
        void update(const FillEvent& fevent);
};
