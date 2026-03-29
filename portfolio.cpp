#include "portfolio.hpp"
#include <stdexcept>

Portfolio::Portfolio(std::vector<Position> init_positions, double init_cash): cash(init_cash) {
    for (const Position& pos: init_positions) {
        auto [it, inserted] = positions_by_ticker.emplace(pos.get_ticker(), pos);
        if (!inserted) {
            throw std::runtime_error("Duplicate position for ticker: " + pos.get_ticker());
        }
    }
}

double Portfolio::total_equity() const {
    double eq = cash;
    for (const std::pair<const std::string, Position>& pair : positions_by_ticker) {
        eq+= pair.second.get_quantity() * prices.at(pair.first);
    }
    return eq;
}

double Portfolio::get_cash() const {
    return cash;
}

const Position& Portfolio::get_position(const std::string& ticker) const {
    auto it = positions_by_ticker.find(ticker);
    if (it == positions_by_ticker.end()) {
        throw std::runtime_error("No position found in protfolio for ticker: " + ticker);
    }
    return it->second;
}

void Portfolio::update_prices(const MarketEvent& market_event) {
    prices[market_event.tick.ticker] = market_event.tick.close;
}


void Portfolio::update_position(const FillEvent& fevent) {
    auto it = positions_by_ticker.find(fevent.ticker);
    if (it == positions_by_ticker.end()) {
        throw std::runtime_error("No position found in portfolio for ticker: " + fevent.ticker);
    }

    Trade trade{
        fevent.timestamp,
        fevent.ticker,
        fevent.side,
        fevent.quantity,
        fevent.price
    };

    it->second.update(trade);

    const double notional = fevent.quantity * fevent.price;
    if (fevent.side == TradeSide::Buy) {
        cash -= notional + fevent.fees;
    } else {
        cash += notional - fevent.fees;
    }
}
