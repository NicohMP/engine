#pragma once
#include "position.hpp"
#include "event_types.hpp"
#include <vector>
#include <string>
#include <unordered_map>

class Portfolio {
    private:
        std::unordered_map<std::string,Position> positions_by_ticker;
        double cash;
        std::unordered_map<std::string,double> prices;

    public:
        Portfolio(std::vector<Position> init_positions, double init_cash);
        double total_equity() const;
        double get_cash() const;
        const Position& get_position(const std::string& ticker) const;
        void update_prices(const MarketEvent& market_event);
        void update_position(const FillEvent& fill_event);
};