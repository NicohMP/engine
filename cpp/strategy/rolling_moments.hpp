#pragma once
#include <cstddef>
#include <deque>
#include <optional>
#include <stdexcept>

class RollingMoments1D {
private: 
    std::deque<double> rolling;
    size_t window_;
    double sum_x = 0.0;
    double sum_x2 = 0.0;
public:
    explicit RollingMoments1D(size_t window);
    bool push(double x);          // false if !finite(x), no state change
    void reset();
    bool ready() const;           // size == window
    size_t size() const;
    size_t window() const;

    double mean() const;          // require ready()
    double var_pop() const;       // ddof=0, require ready()
    double std_pop() const;       // require ready()
    std::optional<double> z_last() const; // nullopt if !ready or std==0
};
class RollingMoments2D {
private:
    size_t window_;
    std::deque<double> rolling_x;
    std::deque<double> rolling_y;
    double sum_xy = 0.0;
    double sum_x = 0.0;
    double sum_y = 0.0;
    double sum_x2 = 0.0;
    double sum_y2 = 0.0;
public:
    explicit RollingMoments2D(size_t window);
    bool push(double x, double y); 
    void reset();
    bool ready() const;
    size_t size() const;
    size_t window() const;

    double cov_pop() const;       // ddof=0
    std::optional<double> corr() const;      // nullopt if zero variance
    std::optional<double> beta_y_on_x() const; // cov(x,y)/var(x), nullopt if var(x)==0
};
