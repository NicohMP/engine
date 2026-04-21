#pragma once
#include "position.hpp"
#include "core/event_types.hpp"
#include <vector>
#include <string>
#include <unordered_map>

class Portfolio {
    private:
        std::unordered_map<SymbolId,Position> positions_by_id; // mapping symbol id <-> positions for O(1) lookups
        double cash; // Available cash in portfolio
        std::unordered_map<SymbolId,double> prices; // map `symbol id`/asset price updated at each next MarketEvent for up to date equity compute

    public:
        Portfolio(std::vector<Position> init_positions, double init_cash);
        // Update positions and prices in the portfolio
        void update_position(const FillEvent& fill_event);
        void update_prices(const MarketEvent& market_event);
        
        // Check if position or price exists for given symbol id
        bool has_price(const SymbolId& ticker) const;
        bool has_position(const SymbolId& ticker) const;
        
        // Raw porftolio arguments
        double get_cash() const;
        double price(const SymbolId& ticker) const;
        const std::unordered_map<SymbolId,Position>& positions() const;
        double position_qty(const SymbolId& ticker) const;
        const Position& get_position(const SymbolId& ticker) const;
        
        // Portfolio statistics
        double total_equity() const;
        std::unordered_map<SymbolId,double> weights() const;
        double gross_exposure() const;
        double net_exposure() const;
        
        
};