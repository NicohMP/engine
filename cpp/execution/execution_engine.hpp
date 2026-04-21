#pragma once
#include "core/event_types.hpp"
#include <vector>

class ExecutionEngine {
    private :
        std::vector<OrderEvent> pending_orders;
        double fee_rate_;
        void make_fill(std::vector<FillEvent>& fills,const OrderEvent& order,const MarketEvent& mevent);

    public:
        explicit ExecutionEngine(double fee_rate = 0.0010);
        void pass_order(const OrderEvent& order);
        std::vector<FillEvent> fill_orders(const MarketEvent& mevent);
        void reset_pending_orders();
};
