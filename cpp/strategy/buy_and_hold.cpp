#include "buy_and_hold.hpp"
#include <unordered_set>
#include <stdexcept>
#include <cmath>
#include <algorithm>


BuyHoldStrategy::BuyHoldStrategy(
    std::vector<std::string> universe,
    std::optional<std::unordered_map<std::string,double>> distribution)
    {
        // Remove duplicates from universe
        std::unordered_set<std::string> seen;
        for (const std::string& ticker : universe) {
            if (seen.insert(ticker).second) {
                universe_.push_back(ticker);
            }
        }
        // Check if universe is empty
        if (universe_.empty()) {
            throw std::runtime_error("Universe cannot be empty");
        }

        if (distribution){
            // Main check : universe and distribution have the same tickers
            bool same_keys = distribution->size() == universe_.size();
            for (const auto& [ticker, weight] : *distribution) {
                if (std::find(universe_.begin(), universe_.end(), ticker) == universe_.end()) {
                    same_keys = false;
                    break;
                }
            }
            // Second check : does distribution sum to 1 
            double weight_sum = 0.0;
            for (const auto& [ticker, weight] : *distribution) {
                weight_sum += weight;
            }
            bool sum_to_1 = is_close(weight_sum,1.0);
            // Reject if condition not met
            if (!(same_keys && sum_to_1)){
                throw std::runtime_error("Distribution size must match universe size");
            }
            // Otherwise accept distribution
            weights_ = *distribution;
        } else { // Default to 1/(#tickers)
            for (const std::string& ticker: universe_){
                weights_[ticker] = 1.0 / universe_.size();
            }
        }
    }


std::optional<std::vector<OrderEvent>> BuyHoldStrategy::on_market_event(const MarketEvent& mevent, const Portfolio& portfolio) {
    if (invested_){
        return std::nullopt;
    }
    // While prices are missing for assets in our universe, wait 
    for (const std::string& ticker: universe_){
        if (!portfolio.has_price(ticker)){
            return std::nullopt;
        }
    }
    // For each asset considered, if price is stored in portfolio,
    // create order to reach target distribution
    std::vector<OrderEvent> orders;
    for (const std::string& ticker: universe_){
        double target_not = weights_[ticker] * portfolio.total_equity();
        double current_qty = 0.0;
        if (portfolio.has_position(ticker)){
            current_qty = portfolio.position_qty(ticker);
        }
        std::optional<OrderEvent> order = build_order_to_target_notional(ticker,portfolio.price(ticker),target_not, current_qty);
        if (order) {
            orders.push_back(*order);
        }
    }

    // If former positions on unwanted assets, flat them
    for (const auto& [ticker, position] : portfolio.positions()) {
        if (std::find(universe_.begin(), universe_.end(), ticker) == universe_.end()) {
            std::optional<OrderEvent> order = flat_asset(ticker, position.get_quantity());
            // Need to check if order is not None.
            if (order) {
                orders.push_back(*order); // Pass *order bc order is an optional, so we want a pointer:refernce to the value inside it when it's not None. 
            }
        }
    }
    invested_ = true;
    // If orders is empty, we want to return an std::nullopt,
    // not an optional containing an empty vector 
    if (orders.empty()) {
        return std::nullopt;
    }
    return orders;

}
