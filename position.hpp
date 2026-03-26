#pragma once

#include <string>
#include "trading_types.hpp"

class Position {
    private:
        std::string ticker;
        double quantity;

    public:
        Position(const std::string& ticker, double initial_quantity);
        double get_quantity() const { return quantity; }
        std::string get_ticker() const { return ticker; }
        void update(const Trade& trade);
};
