#include "buy_and_hold.hpp"
#include <unordered_set>
#include <unordered_map>
#include <stdexcept>
#include <cmath>
#include <algorithm>


BuyHoldStrategy::BuyHoldStrategy(
    std::vector<SymbolId> u_ids,
    std::optional<std::unordered_map<SymbolId,double>> distribution)
    {
        // Remove duplicates from universe
        std::unordered_set<SymbolId> seen;
        for (const SymbolId& id : u_ids) {
            if (seen.insert(id).second) {
                universe_ids.push_back(id);
            }
        }
        // Check if universe is empty
        if (universe_ids.empty()) {
            throw std::runtime_error("Universe cannot be empty");
        }

        if (distribution){
            // Main check : universe and distribution have the same ids after deduplication
            bool same_keys = distribution->size() == u_ids.size();
            for (const auto& [id, weight] : *distribution) {
                if (std::find(universe_ids.begin(), universe_ids.end(), id) == universe_ids.end()) {
                    same_keys = false;
                    break;
                }
            }
            // Second check : does distribution sum to 1 
            double weight_sum = 0.0;
            for (const auto& [id, weight] : *distribution) {
                weight_sum += weight;
            }
            bool sum_to_1 = is_close(weight_sum,1.0);
            // Reject if condition not met
            if (!(same_keys && sum_to_1)){
                throw std::runtime_error("Distribution size must match universe size");
            }
            // Otherwise accept distribution
            weights_ = *distribution;
        } else { // Default to 1/(#assets)
            for (const SymbolId& id: universe_ids){
                weights_[id] = 1.0 / universe_ids.size();
            }
        }
    }


std::optional<std::vector<OrderEvent>> BuyHoldStrategy::on_market_event(
    const MarketEvent& mevent,
    const Portfolio& portfolio,
    const FeatureView& features
) {
    (void)features;
    if (invested_){
        return std::nullopt;
    }
    // While prices are missing for assets in our universe, wait 
    for (const SymbolId& id: universe_ids){
        if (!portfolio.has_price(id)){
            return std::nullopt;
        }
    }
    // For each asset considered, if price is stored in portfolio,
    // create order to reach target distribution
    std::vector<OrderEvent> orders;
    for (const SymbolId& id: universe_ids){
        double target_not = weights_[id] * portfolio.total_equity();
        double current_qty = 0.0;
        if (portfolio.has_position(id)){
            current_qty = portfolio.position_qty(id);
        }
        std::optional<OrderEvent> order = build_order_to_target_notional(id,portfolio.price(id),target_not, current_qty);
        if (order) {
            orders.push_back(*order);
        }
    }

    // If former positions on unwanted assets, flat them
    for (const auto& [id, position] : portfolio.positions()) {
        if (std::find(universe_ids.begin(), universe_ids.end(), id) == universe_ids.end()) {
            std::optional<OrderEvent> order = flat_asset(id, position.get_quantity());
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

std::optional<std::vector<OrderEvent>> BuyHoldStrategy::on_time_slice(
    const std::vector<MarketEvent>& events,
    const Portfolio& portfolio,
    const std::vector<FeatureView>& features
) {
    (void)features;

    if (invested_) {
        return std::nullopt;
    }

    std::unordered_map<SymbolId, const MarketEvent*> event_by_symbol;
    event_by_symbol.reserve(events.size());

    for (const MarketEvent& event : events) {
        event_by_symbol[event.symbol_id] = &event;
    }

    for (const SymbolId& id : universe_ids) {
        if (event_by_symbol.find(id) == event_by_symbol.end()) {
            return std::nullopt;
        }
    }

    std::vector<OrderEvent> orders;

    for (const SymbolId& id : universe_ids) {
        const MarketEvent& event = *event_by_symbol.at(id);

        double target_notional = weights_.at(id) * portfolio.total_equity();
        double current_qty = 0.0;

        if (portfolio.has_position(id)) {
            current_qty = portfolio.position_qty(id);
        }

        std::optional<OrderEvent> order = build_order_to_target_notional(
            id,
            event.close,
            target_notional,
            current_qty
        );

        if (order) {
            orders.push_back(*order);
        }
    }

    for (const auto& [id, position] : portfolio.positions()) {
        if (std::find(universe_ids.begin(), universe_ids.end(), id) == universe_ids.end()) {
            std::optional<OrderEvent> order = flat_asset(id, position.get_quantity());

            if (order) {
                orders.push_back(*order);
            }
        }
    }

    invested_ = true;

    if (orders.empty()) {
        return std::nullopt;
    }

    return orders;
}




void BuyHoldStrategy::reset(){
    invested_ = false;
}
