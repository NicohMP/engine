#include "portfolio.hpp"
#include <stdexcept>
#include <cmath>

Portfolio::Portfolio(std::vector<Position> init_positions, double init_cash): cash(init_cash) {
    for (const Position& pos: init_positions) {
        auto [it, inserted] = positions_by_id.emplace(pos.get_id(), pos);
        if (!inserted) {
            throw std::runtime_error("Duplicate position for id: " + std::to_string(pos.get_id()));
        }
    }
}

void Portfolio::update_prices(const MarketEvent& market_event) {
    prices[market_event.symbol_id] = market_event.close;
}

void Portfolio::update_position(const FillEvent& fevent) {
    auto it = positions_by_id.find(fevent.symbol_id);
    // If position doesn't exist yet, build it and assign it in positions_by_id
    if (it == positions_by_id.end()) {
        double sign = (fevent.side == TradeSide::Buy) ? 1.0 : -1.0;
        positions_by_id.emplace(
            fevent.symbol_id, 
            Position(fevent.symbol_id, sign*fevent.quantity)
        ); 
    }
    else {
        it->second.update(fevent);
    }
    prices[fevent.symbol_id]=  fevent.price; // Update or create price log
    // Update cash in porftolio
    const double notional = fevent.quantity * fevent.price;
    if (fevent.side == TradeSide::Buy) {
        cash -= notional + fevent.fees;
    } else {
        cash += notional - fevent.fees;
    }
}


double Portfolio::get_cash() const {return cash;}

const Position& Portfolio::get_position(const SymbolId& id) const {
    auto it = positions_by_id.find(id);
    if (it == positions_by_id.end()) {
        throw std::runtime_error("No position found in protfolio for id: " + std::to_string(id));
    }
    return it->second;
}

const std::unordered_map<SymbolId,Position>& Portfolio::positions() const {
    return positions_by_id;
}

double Portfolio::total_equity() const {
    double eq = cash;
    for (const std::pair<const SymbolId, Position>& pair : positions_by_id) {
        eq+= pair.second.get_quantity() * prices.at(pair.first);
    }
    return eq;
}

double Portfolio::position_qty(const SymbolId& id) const {
    auto it = positions_by_id.find(id);
    if (it == positions_by_id.end()) {
        throw std::runtime_error("No position found for "+std::to_string(id)+" in portfolio");
    }
    // it is a iterator, that looks like pair<string,Position>, so if I want to access the value, 
    // i need to do it->second because it is more like a pointer to the pair. it is short for (*it).second
    double qty = it->second.get_quantity();
    return qty;
}

std::unordered_map<SymbolId,double> Portfolio::weights() const {
    std::unordered_map<SymbolId,double> weights; // Declare map
    weights.reserve(positions_by_id.size());    // We can already reserve memory for the nb of positions
    const double eqty = total_equity();             // Set the constant total equity to compute weights
    if (eqty <= 0.0) {
        throw std::runtime_error("Portfolio equity must be positive to compute weights");
    }
    for (const auto& [id,pos]: positions_by_id){
        double w = pos.get_quantity()*prices.at(id) / eqty;
        weights.emplace(id,w);
    }
    return weights;
}

double Portfolio::gross_exposure() const {
    double gross =0.0;
    for (const auto& [id,pos]: positions_by_id){
        gross += std::abs(pos.get_quantity()*prices.at(id));
    }
    return gross;
}

double Portfolio::net_exposure() const {
    double net = 0.0;
    for (const auto& [id,pos]: positions_by_id){
        net += pos.get_quantity()*prices.at(id);
    }
    return net;
}

double Portfolio::price(const SymbolId& id) const {
    auto it = prices.find(id);
    if (it == prices.end()) {
        throw std::runtime_error("No price logged for"+std::to_string(id)+"in portfolio");
    }
    // it is a iterator, that looks like pair<string,Position>, so if I want to access the value, 
    // i need to do it->second because it is more like a pointer to the pair. it is short for (*it).second
    double t_price = it->second;
    return t_price;
}

bool Portfolio::has_price(const SymbolId& id) const {
    auto it = prices.find(id);
    bool res =  (it == prices.end());
    return !res;
}


bool Portfolio::has_position(const SymbolId& id) const {
    auto it = positions_by_id.find(id);
    bool res = (it == positions_by_id.end()); // false if the key exists
    return !res;
}