#include "rolling_moments.hpp"
#include <algorithm>
#include <cmath>

RollingMoments1D::RollingMoments1D(size_t window) : window_(window) {
    if (window_ == 0) {
        throw std::runtime_error("RollingMoments1D window must be > 0");
    }
}

bool RollingMoments1D::push(double x){       // false if !finite(x), no state change
    if (!std::isfinite(x)) {
        return false;
    }
    if (ready()){
        double old = rolling.front();
        rolling.pop_front();
        sum_x += x - old;
        sum_x2 += x*x - old*old;
        rolling.push_back(x);
        return true;
    }
    sum_x += x;
    sum_x2 += x*x;
    rolling.push_back(x);
    return true;
}         
void RollingMoments1D::reset(){
    rolling.clear();
    sum_x = 0.0;
    sum_x2 = 0.0;
}

bool RollingMoments1D::ready() const {                          // size == window
    return (rolling.size() == window_);
}   
size_t RollingMoments1D::size() const{
    return rolling.size();
}
size_t RollingMoments1D::window() const{
    return window_;
}

double RollingMoments1D::mean() const{                          // require ready()
    if (!ready()){
        throw std::runtime_error("Rolling_window isn't ready (incorrect nb of values)");
    }
    return sum_x/size();
}

double RollingMoments1D::var_pop() const{                       // ddof=0, require ready()
    if (!ready()){
        throw std::runtime_error("Rolling_window isn't ready (incorrect nb of values)");
    }
    const double n = static_cast<double>(size());
    const double mu = sum_x / n;
    const double var = (sum_x2 / n) - (mu * mu);
    return std::max(0.0, var);
}
double RollingMoments1D::std_pop() const{                       // require ready()
    return std::sqrt(var_pop());
}
std::optional<double> RollingMoments1D::z_last() const{         // nullopt if !ready or std==0
    if (!ready()){
        return std::nullopt;
    }
    const double sigma = std_pop();
    if (sigma == 0.0) {
        return std::nullopt;
    }
    return (rolling.back() - mean()) / sigma;
}


RollingMoments2D::RollingMoments2D(size_t window) : window_(window) {
    if (window_ == 0) {
        throw std::runtime_error("RollingMoments2D window must be > 0");
    }
}
void RollingMoments2D::reset(){
    rolling_x.clear();
    rolling_y.clear();
    sum_x = 0.0;
    sum_y = 0.0;
    sum_xy = 0.0;
    sum_x2 = 0.0;
    sum_y2 = 0.0;
}

bool RollingMoments2D::ready() const {
    return (rolling_x.size()==window_ && rolling_y.size()==window_);
}

bool RollingMoments2D::push(double x, double y){
    if (!std::isfinite(x) || !std::isfinite(y)) {
        return false;
    }
    if (ready()){
        double old_x = rolling_x.front();
        double old_y = rolling_y.front();
        rolling_x.pop_front();
        rolling_y.pop_front();
        sum_x += x - old_x;
        sum_y += y - old_y;
        sum_x2 += x*x - old_x*old_x;
        sum_y2 += y*y - old_y*old_y;
        sum_xy += x*y - old_x*old_y;
        rolling_x.push_back(x);
        rolling_y.push_back(y);
        return true;
    }
    sum_x += x;
    sum_y += y;
    sum_x2 += x*x;
    sum_y2 += y*y;
    sum_xy += x*y;
    rolling_x.push_back(x);
    rolling_y.push_back(y);
    return true;
}

size_t RollingMoments2D::size() const{
    return rolling_x.size();
}

size_t RollingMoments2D::window() const{
    return window_;
}

double RollingMoments2D::cov_pop() const{
    if (!ready()){
        throw std::runtime_error("Rolling_window isn't ready (incorrect nb of values)");
    }
    const double n = static_cast<double>(size());
    const double mu = sum_x / n;
    const double lambda = sum_y / n;
    const double cov = (sum_xy / n) - (mu * lambda);
    return cov;
}

std::optional<double> RollingMoments2D::corr() const{
    if (!ready()){
        return std::nullopt;
    }
    const double cov = cov_pop();
    const double n = static_cast<double>(size());
    const double mu = sum_x / n;
    const double lambda = sum_y / n;
    const double var_x = std::max(0.0, (sum_x2 / n) - (mu * mu));
    const double var_y = std::max(0.0, (sum_y2 / n) - (lambda * lambda));
    if (var_x == 0.0 || var_y == 0.0) {
        return std::nullopt;
    }
    const double std_x = std::sqrt(var_x);
    const double std_y = std::sqrt(var_y);
    return cov / (std_x * std_y);
}   

std::optional<double> RollingMoments2D::beta_y_on_x() const{
    if (!ready()){
        return std::nullopt;
    }
    const double cov = cov_pop();
    const double n = static_cast<double>(size());
    const double mu = sum_x / n;
    const double var_x = std::max(0.0, (sum_x2 / n) - (mu * mu));
    if (var_x == 0.0) {
        return std::nullopt;
    }
    return cov / var_x; 
}
