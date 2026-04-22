# pragma once
#include "base.hpp"
#include "core/event_types.hpp"
#include <vector>
#include <string>
#include <optional>
#include <random> 

class RandomStrategy : public Strategy {
    private:
        std::vector<SymbolId> universe_ids;
        double target_not_;
        std::mt19937 rng_;
        std::vector<double> probas_;

    public:
        RandomStrategy(
            std::vector<SymbolId> universes_ids,
            double target_notional,
            std::optional<std::vector<double>> probabilities = std::nullopt,
            unsigned int seed = 42);
        
        virtual std::optional<std::vector<OrderEvent>> on_market_event(
            const MarketEvent& mevent, 
            const Portfolio& Portfolio) override;

        virtual void reset() override {};

            
};
