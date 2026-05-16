#include "random.hpp"
#include <unordered_set>
#include <stdexcept>
#include <cmath>
#include <algorithm>


RandomStrategy::RandomStrategy(
    std::vector<SymbolId> u_ids,
    double target_notional,
    std::optional<std::vector<double>> probabilities,
    unsigned int seed): rng_(seed){
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
        // 
        // Check if the probas contains 3 values that sum to one 
        if (probabilities){
            double p_sum = 0.0;
            for (const double& p : *probabilities) {
                p_sum += p;
            }
            bool sum_to_1 = is_close(p_sum,1.0);
            if (!((probabilities->size()==3) && sum_to_1)){
                throw std::runtime_error("Distribution must contains three elements and sum to 1.");
            }
            probas_ = *probabilities;
            } else {    // Default to uniform dist on 3 values
                probas_ = {1.0/3.0,1.0/3.0,1.0/3.0};
            }
        // Check that target notional is positive
        if (target_notional <= 0.0){
            throw std::runtime_error("Target notional must be positive");
        }
        target_not_ = target_notional;
    }

std::optional<std::vector<OrderEvent>> RandomStrategy::on_market_event(
    const MarketEvent& mevent,
    const Portfolio& portfolio,
    const FeatureView& features
) {
    (void)features;
    if (std::find(universe_ids.begin(), universe_ids.end(), mevent.symbol_id) == universe_ids.end()){
        return std::nullopt;
    }
    std::discrete_distribution<int> dist(probas_.begin(), probas_.end()); // Random distribution in {0,1,2}
    int signal = dist(rng_) -1; // random variable in {-1,0,1}
    if (signal ==0){
        return std::nullopt;
    }
    double current_qty = 0.0;
    if (portfolio.has_position(mevent.symbol_id)){
        current_qty = portfolio.position_qty(mevent.symbol_id);
    }
    std::optional<OrderEvent> order = build_order_to_target_notional(
        mevent.symbol_id,mevent.close,signal*target_not_,current_qty);
    std::vector<OrderEvent> orders;
    if (!order){
        return std::nullopt;
    }
    orders.push_back(*order);
    return orders;

            
}

std::optional<std::vector<OrderEvent>> RandomStrategy::on_time_slice(
    const std::vector<MarketEvent>& events,
    const Portfolio& portfolio,
    const std::vector<FeatureView>& features
) {
    if (events.size() != features.size()) {
        throw std::runtime_error("events and features must have the same size");
    }

    std::vector<OrderEvent> batch;

    for (size_t i = 0; i < events.size(); ++i) {
        auto maybe_orders = on_market_event(events[i], portfolio, features[i]);

        if (maybe_orders) {
            batch.insert(batch.end(), maybe_orders->begin(), maybe_orders->end());
        }
    }

    if (batch.empty()) {
        return std::nullopt;
    }

    return batch;
}

