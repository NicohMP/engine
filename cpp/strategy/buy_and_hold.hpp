#pragma once
#include "base.hpp"
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class BuyHoldStrategy : public Strategy {
    private: 
        std::vector<std::string> universe_;                 // Vector of tickers considered in the trading universe of the strategy
        std::unordered_map<std::string,double> weights_;    // Weight distribution of the portfolio equity between ticker of the universe
        bool invested_ = false;                             // Wether the initial investement have been made ot not

    public:
        BuyHoldStrategy(
            std::vector<std::string> universe_,
            std::optional<std::unordered_map<std::string,double>> distribution = std::nullopt);
        virtual std::optional<std::vector<OrderEvent>> on_market_event(const MarketEvent& mevent, const Portfolio& Portfolio) override;    
};