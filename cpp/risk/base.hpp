#pragma once
#include <optional>
#include <vector>
#include "event_types.hpp"
#include "portfolio.hpp"


class RiskManagerBase {
public:
    virtual ~RiskManagerBase() = default;

    virtual std::optional<std::vector<OrderEvent>> accept_order_batch(
        const std::vector<OrderEvent>& orders,
        const Portfolio& portfolio
    )const  = 0;
};
