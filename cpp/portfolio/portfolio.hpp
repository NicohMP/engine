#pragma once
#include "position.hpp"
#include "event_types.hpp"
#include <vector>
#include <string>
#include <unordered_map>

class Portfolio {
    private:
        std::unordered_map<std::string,Position> positions_by_ticker; // mapping ticker <-> positions for O(1) lookups
        double cash; // Available cash in portfolio
        std::unordered_map<std::string,double> prices; // map `ticker`/asset price updated at each next MarketEvent for up to date equity compute

    public:
        Portfolio(std::vector<Position> init_positions, double init_cash);
        // Update positions and prices in the portfolio
        void update_position(const FillEvent& fill_event);
        void update_prices(const MarketEvent& market_event);
        // Raw porftolio arguments
        double get_cash() const;
        const Position& get_position(const std::string& ticker) const;
        // Portfolio statistics
        double total_equity() const;
        std::unordered_map<std::string,double> weights() const;
        double gross_exposure() const;
        double net_exposure() const;
        double price(const std::string& ticker) const;
        double position_qty(const std::string& ticker) const;
};