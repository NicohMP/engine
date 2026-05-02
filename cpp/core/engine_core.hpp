#pragma once

#include "event_types.hpp"
#include "execution/execution_engine.hpp"
#include "portfolio/portfolio.hpp"
#include "risk/base.hpp"
#include "strategy/base.hpp"
#include "data/features/feature_config.hpp"
#include "data/features/feature_engine.hpp"

#include <deque>
#include <memory>
#include <optional>
#include <utility>
#include <vector>



class EngineCore {
    private:
        static constexpr double kFeeRateStandard = 0.0010;  // 0.10%
        static constexpr double kFeeRateDiscount = 0.00075; // 0.075%

        const std::vector<SymbolId> universe_;
        const Portfolio initial_portfolio;
        Portfolio portfolio;
        std::shared_ptr<RiskManagerBase> risk_manager_; // shared pointer is automatically nullptr when nothig is passed so no need for std::optional<> anymore
        std::shared_ptr<Strategy> strategy_;
        FeatureConfig feature_config_;
        FeatureEngine feat_engine_;
        ExecutionEngine execution_engine_;
        std::deque<Event> event_queue_;
        std::vector<OrderEvent> orders_;
        std::vector<FillEvent> fills_;
        std::vector<std::pair<int64_t, double>> equity_;
        double fee_rate_;
    public:
        EngineCore(
            std::vector<SymbolId> universe,
            std::shared_ptr<Strategy> strategy,
            Portfolio portfolio,
            FeatureConfig feature_config,
            std::shared_ptr<RiskManagerBase> risk_manager = nullptr,
            double fee_rate = kFeeRateStandard
        );
        EngineCore(
            std::vector<SymbolId> universe,
            std::shared_ptr<Strategy> strategy,
            Portfolio portfolio,
            FeatureConfig feature_config,
            double fee_rate
        );
        void step(const MarketEvent& mevent);
        void reset_runtime_logs();
        void reset_step_state();
        void reset_all();
        const std::vector<std::pair<int64_t, double>>& get_equity() const { return equity_; }
        const std::vector<OrderEvent>& get_orders() const { return orders_; }
        const std::vector<FillEvent>& get_fills() const { return fills_; }
        const Portfolio& get_portfolio() const { return portfolio;}
        const std::vector<std::string>& get_features() const;
        double get_fee_rate() const { return fee_rate_; }

};
