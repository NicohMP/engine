#pragma once
#include "base.hpp"
#include "core/event_types.hpp"
#include <optional>
#include <unordered_map>
#include <vector>

class BuyHoldStrategy : public Strategy {
    private: 
        std::vector<SymbolId> universe_ids;
        std::unordered_map<SymbolId,double> weights_;
        bool invested_ = false;                             // Wether the initial investement have been made ot not

    public:
        BuyHoldStrategy(
            std::vector<SymbolId> u_ids,
            std::optional<std::unordered_map<SymbolId,double>> distribution = std::nullopt);
        virtual std::optional<std::vector<OrderEvent>> on_market_event(
            const MarketEvent& mevent,
            const Portfolio& Portfolio,
            const FeatureView& features) override;   
        virtual void reset() override ;
};
