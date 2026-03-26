#include "backtester.hpp"


Backtester::Backtester(std::string ticker,const std::vector<Tick>& ticks,std::unique_ptr<Strategy> strategy, AccountState init_account_state): ticker_(ticker), strategy_(std::move(strategy)), position_(Position(ticker_, 0.0)){
    for (const Tick& tick : ticks) {
        if (tick.ticker == ticker_) {
            ticks_.push_back(tick);
        }
    }
    if (ticks_.empty()){
        throw std::runtime_error("no data for ticker " + ticker_);
    }
    std::sort(ticks_.begin(), ticks_.end(), [](const Tick& a, const Tick& b){return a.timestamp < b.timestamp;});
    account_state_ = init_account_state;
    equity_.reserve(ticks_.size());
}


void Backtester::run() {
    for (const Tick& tick : ticks_) {
        Action action = strategy_->on_tick(tick, position_, account_state_);
        if (action.type == ActionType::Buy){
            Trade trade{tick.timestamp, ticker_, TradeSide::Buy, *action.quantity, tick.close};
            trades_.push_back(trade);
            position_.update(trade);
            account_state_.cash -= trade.quantity*tick.close;

        } else if (action.type == ActionType::Sell) {
            Trade trade{tick.timestamp, ticker_, TradeSide::Sell, *action.quantity, tick.close};
            trades_.push_back(trade);
            position_.update(trade);
            account_state_.cash += trade.quantity*tick.close;
            
        }
        account_state_.equity = account_state_.cash + position_.get_quantity()*tick.close;
        equity_.push_back({tick.timestamp, account_state_.equity});
    }
}