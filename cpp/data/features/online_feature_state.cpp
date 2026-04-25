#include "online_feature_state.hpp"
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>

OnlineFeatureState::OnlineFeatureState(const SymbolId& id, const size_t& window)
    : ret_moments_(window), price_moments_(window), id_(id) {}

bool OnlineFeatureState::push(const MarketEvent& mevent){
    if (mevent.symbol_id != id_){
        throw std::runtime_error("Incorrect id "+ std::to_string(mevent.symbol_id)+" passed, expected "+ std::to_string(id_));
    }

    if (!std::isfinite(mevent.close) || mevent.close <= 0.0) {
        throw std::runtime_error("Invalid close for online feature state");
    }

    (void)price_moments_.push(mevent.close);

    if (!has_last_close_) {
        last_close_ = mevent.close;
        has_last_close_ = true;
        return false;
    }

    last_ret_ = mevent.close / last_close_ - 1.0;
    has_last_ret_ = true;
    last_close_ = mevent.close;
    return ret_moments_.push(last_ret_);
}
bool OnlineFeatureState::ready() const{
    return ret_moments_.ready();
}

double OnlineFeatureState::ret_1d() const{
    if (has_last_ret_){
        return last_ret_;
    }
    throw std::runtime_error("No return stored yet");
}
double OnlineFeatureState::vol_pop() const{
    return ret_moments_.var_pop();
}
std::optional<double> OnlineFeatureState::price_z() const{
    return price_moments_.z_last();
}

FeatureRow OnlineFeatureState::to_feature_row_ret_1d() const {
    const double kNaN = std::numeric_limits<double>::quiet_NaN();
    return FeatureRow{
        id_,
        {has_last_ret_ ? last_ret_ : kNaN}
    };
}
