#include "portfolio.hpp"
#include <stdexcept>
#include <cmath>

Portfolio::Portfolio(std::vector<Position> init_positions, double init_cash): cash(init_cash) {
    for (const Position& pos: init_positions) {
        auto [it, inserted] = positions_by_ticker.emplace(pos.get_ticker(), pos);
        if (!inserted) {
            throw std::runtime_error("Duplicate position for ticker: " + pos.get_ticker());
        }
    }
}

void Portfolio::update_prices(const MarketEvent& market_event) {
    prices[market_event.ticker] = market_event.close;
}

void Portfolio::update_position(const FillEvent& fevent) {
    auto it = positions_by_ticker.find(fevent.ticker);
    // If position doesn't exist yet, build it and assign it in positions_by_ticker
    if (it == positions_by_ticker.end()) {
        double sign = (fevent.side == TradeSide::Buy) ? 1.0 : -1.0;
        positions_by_ticker.emplace(
            fevent.ticker, 
            Position(fevent.ticker, sign*fevent.quantity)
        ); 
    }
    else {
        it->second.update(fevent);
    }
    prices[fevent.ticker]=  fevent.price; // Update or create price log
    // Update cash in porftolio
    const double notional = fevent.quantity * fevent.price;
    if (fevent.side == TradeSide::Buy) {
        cash -= notional + fevent.fees;
    } else {
        cash += notional - fevent.fees;
    }
}


double Portfolio::get_cash() const {return cash;}

const Position& Portfolio::get_position(const std::string& ticker) const {
    auto it = positions_by_ticker.find(ticker);
    if (it == positions_by_ticker.end()) {
        throw std::runtime_error("No position found in protfolio for ticker: " + ticker);
    }
    return it->second;
}

double Portfolio::total_equity() const {
    double eq = cash;
    for (const std::pair<const std::string, Position>& pair : positions_by_ticker) {
        eq+= pair.second.get_quantity() * prices.at(pair.first);
    }
    return eq;
}

double Portfolio::position_qty(const std::string& ticker) const {
    auto it = positions_by_ticker.find(ticker);
    if (it == positions_by_ticker.end()) {
        throw std::runtime_error("No position found for "+ticker+" in portfolio");
    }
    // it is a iterator, that looks like pair<string,Position>, so if I want to access the value, 
    // i need to do it->second because it is more like a pointer to the pair. it is short for (*it).second
    double qty = it->second.get_quantity();
    return qty;
}

std::unordered_map<std::string,double> Portfolio::weights() const {
    std::unordered_map<std::string,double> weights; // Declare map
    weights.reserve(positions_by_ticker.size());    // We can already reserve memory for the nb of positions
    const double eqty = total_equity();             // Set the constant total equity to compute weights
    if (eqty <= 0.0) {
        throw std::runtime_error("Portfolio equity must be positive to compute weights");
    }
    for (const auto& [ticker,pos]: positions_by_ticker){
        double w = pos.get_quantity()*prices.at(ticker) / eqty;
        weights.emplace(ticker,w);
    }
    return weights;
}

double Portfolio::gross_exposure() const {
    double gross =0.0;
    for (const auto& [ticker,pos]: positions_by_ticker){
        gross += std::abs(pos.get_quantity()*prices.at(ticker));
    }
    return gross;
}

double Portfolio::net_exposure() const {
    double net = 0.0;
    for (const auto& [ticker,pos]: positions_by_ticker){
        net += pos.get_quantity()*prices.at(ticker);
    }
    return net;
}

double Portfolio::price(const std::string& ticker) const {
    auto it = prices.find(ticker);
    if (it == prices.end()) {
        throw std::runtime_error("No price logged for"+ticker+"in portfolio");
    }
    // it is a iterator, that looks like pair<string,Position>, so if I want to access the value, 
    // i need to do it->second because it is more like a pointer to the pair. it is short for (*it).second
    double t_price = it->second;
    return t_price;
}

