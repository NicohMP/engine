#pragma once
#include "core/event_types.hpp"
#include "core/engine_core.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

struct StepResult {
    std::optional<int64_t> timestamp;
    std::vector<OrderEvent> new_orders;
    std::vector<FillEvent> new_fills;
    double equity_after;
    size_t total_orders;
    size_t total_fills;
};


class Simulator {
    private:
        static constexpr double kFeeRateStandard = 0.0010;  // 0.10%
        static constexpr double kFeeRateDiscount = 0.00075; // 0.075%

        EngineCore engine_;
    
    public:
        Simulator(
            std::vector<SymbolId> universe,
            std::shared_ptr<Strategy> strategy,
            Portfolio portfolio,
            FeatureConfig feature_config,
            std::shared_ptr<RiskManagerBase> risk_manager = nullptr,
            double fee_rate = kFeeRateStandard
        );
        Simulator(
            std::vector<SymbolId> universe,
            std::shared_ptr<Strategy> strategy,
            Portfolio portfolio,
            FeatureConfig feature_config,
            double fee_rate
        );
        StepResult step_bar(const MarketEvent& mevent);
        StepResult step_timestamp(const std::vector<MarketEvent>& slice);
        void reset_runtime();
        
        const Portfolio& portfolio() const{ return engine_.get_portfolio(); };
        const std::vector<std::pair<int64_t, double>>& get_equity() const { return engine_.get_equity(); }
        const std::vector<OrderEvent>& get_orders() const { return engine_.get_orders(); }
        const std::vector<FillEvent>& get_fills() const { return engine_.get_fills(); }
        double get_fee_rate() const { return engine_.get_fee_rate(); }

};
