# pragma once
#include "base.hpp"
#include <vector>
#include <string>
#include <optional>
#include <random> 

class RandomStrategy : public Strategy {
    private:
        std::vector<std::string> universe_; 
        double target_not_;
        std::mt19937 rng_;
        std::vector<double> probas_;

    public:
        RandomStrategy(
            std::vector<std::string> universe,
            double target_notional,
            std::optional<std::vector<double>> probabilities = std::nullopt,
            unsigned int seed = 42);
        
        virtual std::optional<std::vector<OrderEvent>> on_market_event(
            const MarketEvent& mevent, 
            const Portfolio& Portfolio) override;

            
};
