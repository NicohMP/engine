#pragma once
#include "core/engine_core.hpp"
#include "data/store/market_data_store.hpp"
#include "data/features/config/feature_config.hpp"

#include <memory>
#include <utility>
#include <vector>


class Backtester {
    private:
        static constexpr double kFeeRateStandard = 0.0010;  // 0.10%
        static constexpr double kFeeRateDiscount = 0.00075; // 0.075%

        const MarketDataStore& data_;
        EngineCore engine;
        FeatureConfig feature_config_;

    public:
        Backtester(
            const MarketDataStore& data,
            std::shared_ptr<Strategy> strategy,
            Portfolio portfolio,
            FeatureConfig feature_config,
            std::shared_ptr<RiskManagerBase> risk_manager = nullptr,
            double fee_rate = kFeeRateStandard
        );
        Backtester(
            const MarketDataStore& data,
            std::shared_ptr<Strategy> strategy,
            Portfolio portfolio,
            FeatureConfig feature_config,
            double fee_rate
        );
        void run();
        const std::vector<std::pair<int64_t, double>>& get_equity() const { return engine.get_equity(); }
        const std::vector<OrderEvent>& get_orders() const { return engine.get_orders();}
        const std::vector<FillEvent>& get_fills() const { return engine.get_fills(); }
        double get_fee_rate() const { return engine.get_fee_rate(); }
    
};
