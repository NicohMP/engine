#pragma once
#include "core/event_types.hpp"
#include "portfolio/portfolio.hpp"
#include "risk/base.hpp"
#include <optional>
#include <stdexcept>
#include <vector>


struct RiskLimits{
    double max_position_weight = 1.0;
    double max_gross_exposure = 1.0;
    double max_net_exposure = 50.0;
    double max_nb_positions = 1000;
};

Portfolio copy_portfolio(const Portfolio& portfolio);

void apply_hypothetical_order(const OrderEvent& order, Portfolio& portfolio);

class SimpleRiskManager: public RiskManagerBase {
    private:
        RiskLimits limits_;
    public:
        SimpleRiskManager(RiskLimits limits);
        virtual std::optional<std::vector<OrderEvent>> accept_order_batch(const std::vector<OrderEvent>& orders, const Portfolio& portfolio) const override;
        bool acceptable(const Portfolio& portfolio) const;
        virtual void reset() override {};
};
