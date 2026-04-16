#include "execution_engine.hpp"
#include <stdexcept>
#include <utility>

ExecutionEngine::ExecutionEngine(double fee_rate) : fee_rate_(fee_rate) {
    if (fee_rate_ < 0.0) {
        throw std::runtime_error("ExecutionEngine fee_rate must be non-negative");
    }
}

void ExecutionEngine::pass_order(const OrderEvent& order){
    pending_orders.push_back(order);
}

void ExecutionEngine::make_fill(std::vector<FillEvent>& fills,const OrderEvent& order, const MarketEvent& mevent){
    if (!order.quantity || *order.quantity <= 0.0) {
        throw std::runtime_error("Invalid order quantity in ExecutionEngine::fill_orders");
    }

    const double fill_quantity = *order.quantity;
    const double fill_price = mevent.close; // Convention: all fills execute at current bar close.
    const double notional = fill_quantity * fill_price;
    const double fees = notional * fee_rate_;

    fills.push_back(FillEvent{
        order.id,
        mevent.timestamp,
        order.ticker,
        order.side,
        fill_quantity,
        fill_price,
        fees
    });
}

std::vector<FillEvent> ExecutionEngine::fill_orders(const MarketEvent& mevent) {
    std::vector<FillEvent> fills;
    std::vector<OrderEvent> still_pending;
    fills.reserve(pending_orders.size());
    still_pending.reserve(pending_orders.size());

    for (const OrderEvent& order : pending_orders) {
        if (order.ticker != mevent.ticker) { // Convention: this bar can only fill orders for its own symbol.
            still_pending.push_back(order);
            continue;
        }

        switch (order.order_type) {
            case OrderType::Market: {
                // Convention: market orders fill immediately on the current bar.
                make_fill(fills, order, mevent);
                break;
            }

            case OrderType::Limit: {
                if (!order.price) {
                    throw std::runtime_error("Limit order requires price");
                }
                const double limit_price = *order.price;
                const bool limit_hit =
                    // Convention: limit trigger uses bar range (high/low), then fills at bar close.
                    ((order.side == TradeSide::Buy) && (mevent.low <= limit_price)) ||
                    ((order.side == TradeSide::Sell) && (mevent.high >= limit_price));

                if (limit_hit) {
                    make_fill(fills, order, mevent);
                } else {
                    still_pending.push_back(order);
                }
                break;
            }

            case OrderType::Stop: {
                if (!order.price) {
                    throw std::runtime_error("Stop order requires trigger price");
                }
                const double stop_price = *order.price;
                const bool stop_triggered =
                    ((order.side == TradeSide::Buy) && (mevent.high >= stop_price)) ||
                    ((order.side == TradeSide::Sell) && (mevent.low <= stop_price));

                if (stop_triggered) {
                    // Stage 2 convention: stop turns into a market fill on the trigger bar.
                    make_fill(fills, order, mevent);
                } else {
                    still_pending.push_back(order);
                }
                break;
            }
        }
    }

    pending_orders = std::move(still_pending);
    return fills;
}
