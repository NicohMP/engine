#include "online_feature_state.hpp"
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>

// Define Nan type
namespace {
    double nan_value() {
        return std::numeric_limits<double>::quiet_NaN();
    }
}

OnlineFeatureState::OnlineFeatureState(const SymbolId& id, const size_t& window)
    : ret_moments_(window), price_moments_(window), volume_moments_(window), id_(id) {}

bool OnlineFeatureState::push(const MarketEvent& mevent){
    // Check event's id is supported
    if (mevent.symbol_id != id_){
        throw std::runtime_error("Incorrect id "+ std::to_string(mevent.symbol_id)+" passed, expected "+ std::to_string(id_));
    }

    // Check close price is valid
    if (!std::isfinite(mevent.close) || mevent.close <= 0.0) {
        throw std::runtime_error("Invalid close for online feature state");
    }
    if (!std::isfinite(mevent.high) || !std::isfinite(mevent.low) || mevent.high < mevent.low) {
        throw std::runtime_error("Invalid high/low for online feature state");
    }
    if (!std::isfinite(mevent.volume) || mevent.volume < 0.0) {
        throw std::runtime_error("Invalid volume for online feature state");
    }

    // Push close and volume buffers
    (void)price_moments_.push(mevent.close);
    (void)volume_moments_.push(mevent.volume);
    last_high_low_range_ = (mevent.high - mevent.low) / mevent.close;
    has_last_high_low_range_ = true;

    // Handle return warmup
    if (!has_last_close_) {
        last_close_ = mevent.close;
        has_last_close_ = true;
        return false;
    }
  
    // Compute new return and log return values
    last_ret_ = mevent.close / last_close_ - 1.0;
    has_last_ret_ = true;
    last_logret_ = std::log(mevent.close / last_close_);
    has_last_logret_ = true;
    last_close_ = mevent.close;

    return ret_moments_.push(last_ret_);
}

bool OnlineFeatureState::ready() const{
    return ret_moments_.ready();
}

void OnlineFeatureState::reset(){
    last_close_ = 0.0;
    has_last_close_ = false;
    last_ret_ = 0.0;
    has_last_ret_ = false;
    last_logret_ = 0.0;
    has_last_logret_ = false;
    last_high_low_range_ = 0.0;
    has_last_high_low_range_ = false;
    ret_moments_.reset();
    price_moments_.reset();
    volume_moments_.reset();
}

double OnlineFeatureState::feature_ret_1d() const{
    return has_last_ret_ ? last_ret_ : nan_value();
}
double OnlineFeatureState::feature_ret_var() const{
    return ret_moments_.ready() ? ret_moments_.var_pop() : nan_value();
}

double OnlineFeatureState::feature_ret_vol() const{
    return ret_moments_.ready() ? ret_moments_.std_pop() : nan_value();
}

double OnlineFeatureState::feature_price_z() const{
    const std::optional<double> z = price_moments_.z_last();
    return z.has_value() ? *z : nan_value();
}

double OnlineFeatureState::feature_volume_z() const{
    const std::optional<double> z = volume_moments_.z_last();
    return z.has_value() ? *z : nan_value();
}

double OnlineFeatureState::feature_close_to_sma() const{
    return price_moments_.ready() ? (last_close_ / price_moments_.mean()) - 1.0 : nan_value();
}

double OnlineFeatureState::feature_logret_1d() const{
    return has_last_logret_ ? last_logret_ : nan_value();
}

double OnlineFeatureState::feature_high_low_range() const{
    return has_last_high_low_range_ ? last_high_low_range_ : nan_value();
}

double OnlineFeatureState::compute_feature(FeatureId id) const{
    switch (id) {
    case FeatureId::Ret1D:
        return feature_ret_1d();
    case FeatureId::LogRet1D:
        return feature_logret_1d();
    case FeatureId::RetVar:
        return feature_ret_var();
    case FeatureId::RetVol:
        return feature_ret_vol();
    case FeatureId::PriceZ:
        return feature_price_z();
    case FeatureId::VolumeZ:
        return feature_volume_z();
    case FeatureId::HighLowRange:
        return feature_high_low_range();
    case FeatureId::CloseToSma:
        return feature_close_to_sma();
    }
    throw std::runtime_error("Unsupported feature id");
}

FeatureRow OnlineFeatureState::to_feature_row(const FeatureSchema& schema) const {
    // Build feature values vector and allocate memory
    std::vector<double> values;
    values.reserve(schema.size());
    for (const FeatureId& feat:schema.features()){
        values.push_back(compute_feature(feat));
    }
    FeatureRow feat_row{id_, values};
    return feat_row;
}
