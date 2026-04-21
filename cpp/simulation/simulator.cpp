#include "simulator.hpp"
#include <algorithm>
#include <stdexcept>

Simulator::Simulator(
    std::shared_ptr<Strategy> strategy,
    Portfolio init_portfolio,
    std::shared_ptr<RiskManagerBase> risk_manager,
    double fee_rate
    ): 
    engine_(strategy,init_portfolio,risk_manager,fee_rate)
    {
    if (fee_rate < 0.0) {
        throw std::runtime_error("fee_rate must be non-negative");
    }
    // if (data_.empty()){ // check market events isn't empty
    //     throw std::runtime_error("no data ");
    // }    
}

Simulator::Simulator(
    std::shared_ptr<Strategy> strategy,
    Portfolio init_portfolio,
    double fee_rate
) : Simulator(
        std::move(strategy),
        std::move(init_portfolio),
        nullptr, // now that we use a shared_ptr, the None equivalent is nullptr
        fee_rate
    ) {}


void Simulator::reset_runtime(){
    engine_.reset_runtime_logs();
}

StepResult Simulator::step_bar(const MarketEvent& mevent){

    const size_t old_orders_nb = engine_.get_orders().size();
    const size_t old_fills_nb = engine_.get_fills().size();

    engine_.step(mevent);
    
    const std::vector<OrderEvent>& tot_orders = engine_.get_orders();
    const std::vector<FillEvent>& tot_fills = engine_.get_fills();

    StepResult out{};
    out.new_orders.assign(tot_orders.begin() + static_cast<std::ptrdiff_t>(old_orders_nb), tot_orders.end());
    out.new_fills.assign(tot_fills.begin() + static_cast<std::ptrdiff_t>(old_fills_nb), tot_fills.end());
    out.total_orders = tot_orders.size();
    out.total_fills = tot_fills.size();
    out.timestamp = mevent.timestamp;
    out.equity_after = engine_.get_equity().back().second;

    return out;

}

StepResult Simulator::step_timestamp(const std::vector<MarketEvent>& slice){
    /* Process a whole timestamp data for all considered assets*/
    // If slice is empty, return old values, null timestamp, and empty new_orders/fills.
    if (slice.empty()){
        StepResult out{
            std::nullopt,{},{},
            engine_.get_equity().back().second,
            engine_.get_orders().size(),
            engine_.get_fills().size()
        };
        return out;
    }

    // Check that all events have the same timestamp
    int64_t ts = slice.front().timestamp;
    for (const MarketEvent& mevent:slice){
        if (mevent.timestamp != ts) {
            throw std::runtime_error("Inconsistent timestamps "+std::to_string(ts)+" and "+std::to_string(mevent.timestamp));
        }
    }
    // Deterministic processing order inside one timestamp.
    std::vector<MarketEvent> sorted_slice = slice;
    std::stable_sort(
        sorted_slice.begin(),
        sorted_slice.end(),
        [](const MarketEvent& a, const MarketEvent& b) {
            return a.symbol_id < b.symbol_id;
        }
    );

    // Get nb of orders/fills before step
    const size_t old_orders_nb = engine_.get_orders().size();
    const size_t old_fills_nb = engine_.get_fills().size();
    
    // Step engine through timestamp
    for (const MarketEvent& mevent:sorted_slice){
        engine_.step(mevent);
    }
    // Update orders/fills
    const std::vector<OrderEvent>& tot_orders = engine_.get_orders();
    const std::vector<FillEvent>& tot_fills = engine_.get_fills();

    // Fill step result 
    StepResult out{};
    out.new_orders.assign(tot_orders.begin() + static_cast<std::ptrdiff_t>(old_orders_nb), tot_orders.end());
    out.new_fills.assign(tot_fills.begin() + static_cast<std::ptrdiff_t>(old_fills_nb), tot_fills.end());
    out.total_orders = tot_orders.size();
    out.total_fills = tot_fills.size();
    out.timestamp = ts;
    out.equity_after = engine_.get_equity().back().second;

    return out;
}
