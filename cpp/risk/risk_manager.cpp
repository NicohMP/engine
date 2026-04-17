#include "risk_manager.hpp"
#include "portfolio/position.hpp"
#include <optional>

Portfolio copy_portfolio(const Portfolio& portfolio){
    Portfolio shadow = portfolio;
    return shadow;
}


void apply_hypothetical_order(const OrderEvent& order, Portfolio& portfolio){
    if (!order.quantity || *order.quantity <= 0.0) {
        throw std::runtime_error("Invalid order quantity");
    }

    FillEvent fill{
        order.id,
        0,
        order.ticker, 
        order.side,
        *order.quantity, 
        portfolio.price(order.ticker),
        0.0};
    portfolio.update_position(fill);
}

RiskManager::RiskManager(RiskLimits limits) : limits_(limits){}

std::optional<std::vector<OrderEvent>> RiskManager::accept_order_batch(const std::vector<OrderEvent>& orders, const Portfolio& portfolio) const{
    Portfolio shadow = copy_portfolio(portfolio);
    for (const OrderEvent& order:orders){
        apply_hypothetical_order(order, shadow);
    }
    if (!acceptable(shadow)){
        return std::nullopt;
    }
    return orders;
}
        


bool RiskManager::acceptable(const Portfolio& portfolio) const{
    bool decision = true;
    double equity = portfolio.total_equity();
    if (equity <= 0.0) {
        return false;
    }
    double gross = std::abs(portfolio.gross_exposure()/ equity);
    double net = std::abs(portfolio.net_exposure()/ equity);
    if (gross > limits_.max_gross_exposure){decision = false;}
    if (net > limits_.max_net_exposure){decision = false;}
    int non_null_pos = 0;
    for (const auto& [t,pos]: portfolio.positions()){
        double qty = pos.get_quantity();
        if (qty != 0.0) {non_null_pos++;}
    }
    if (non_null_pos > limits_.max_nb_positions){decision = false;}
    
    double max_weight = 0.0;
    for (const auto& [ticker, weight] : portfolio.weights()) {
        if (std::abs(weight) > max_weight) {
            max_weight = std::abs(weight);
        }
    }
    if (max_weight > limits_.max_position_weight){decision = false;}
    return decision;
}
