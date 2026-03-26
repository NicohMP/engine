#include "analytics.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <limits>
#include <stdexcept>
#include <iostream>
using std::vector;
using std::string;
using std::pair;

vector<Tick> per_ticker(const vector<Tick>& ticks, string ticker) {
    vector<Tick> per_ticker;
    for (const Tick& tick : ticks) {
        if (tick.ticker == ticker) {
            per_ticker.push_back(tick);
        }
    }
    return per_ticker;
};

vector<Tick> chronological_sort(vector<Tick> ticks) {
    /**
     * @brief Sorts ticks in chronological order (ascending timestamp).
     *
     * @param ticks Vector of Tick objects (copied, not modified in place)
     * @return A new vector sorted by timestamp
    */
    std::sort(ticks.begin(), ticks.end(), [](const Tick& a, const Tick& b){return a.timestamp < b.timestamp;});
    return ticks;
}

SymbolSeries::SymbolSeries(const vector<Tick>& ticks_, const string& ticker_) : ticker(ticker_) {
    for (const Tick& tick : ticks_) {
        if (tick.ticker == ticker) {
            ticks.push_back(tick);
        }
    }
    if (ticks.empty()){
        throw std::runtime_error("no data for ticker " + ticker_);
    }
    std::sort(ticks.begin(), ticks.end(), [](const Tick& a, const Tick& b){return a.timestamp < b.timestamp;});
}

double SymbolSeries::min_price(double Tick::* field) const {             
    double min_value = std::numeric_limits<double>::infinity();
    for (const Tick& tick : ticks) {
        double value = tick.*field;
        if  (value < min_value) {
            min_value = value;
        }    
    }    
    return min_value;
}    

double SymbolSeries::max_price(double Tick::* field) const {             
    double max_value = std::numeric_limits<double>::lowest();

    for (const Tick& tick : ticks) {
        double value = tick.*field;
        if (value > max_value) {
            max_value = value;
        }    
    }      
    return max_value;
}    

double SymbolSeries::avg_price(double Tick::* field) const {             
    double sum = 0;
    int count = 0;
    for (const Tick& tick : ticks) {
        double value = tick.*field;    
        sum += value;
        count ++;
    }    
    return sum/count;
}    

vector<double> SymbolSeries::mva(double Tick::* field, size_t window) const {

    if (window > ticks.size()) { // handling window too large
        throw std::runtime_error("Window size can't be greater than number of ticks ");
    } else if (window == 0) {
        throw std::runtime_error("Window size can't be 0");
    }
    vector<double> mvas; // declare vector of SMA
    mvas.reserve(ticks.size() - window + 1);  // single memory allocation for SMAs, avoids dynamic memory allocation when push_back --> more efficient. 
    double sum = std::accumulate(ticks.begin(),ticks.begin()+window,0.0, [&](double acc, const Tick& t){return acc + t.*field;}); // compute first sum from 0 to window-1
    mvas.push_back(sum/window); // store 1st SMA
    size_t l = 0;
    size_t r = window;
    while (r < ticks.size())    {
        sum += ticks[r].*field;
        sum -= ticks[l].*field;
        l++;
        r++;
        mvas.push_back(sum/window);
    }
    return mvas;
}

BacktestReport compute_backtest_report(const vector<pair<int64_t, double>>& equity, const vector<Trade>& trades) {
    if (equity.empty()) {
        throw std::runtime_error("Equity log is empty");
    }
    BacktestReport report;
    report.final_equity = equity.back().second;
    report.total_return = (equity.back().second/equity.front().second) -1;
   
    // compute volatility
    std::vector<double> returns;
    if (equity.size() >= 2) {
        returns.reserve(equity.size() - 1);
        for (size_t i = 1; i < equity.size(); ++i) {
            double prev = equity[i - 1].second;
            double curr = equity[i].second;
            // simple return
            returns.push_back((curr / prev) - 1.0);
        }
    
        double sum_returns = 0.0;
        for (const double& ret : returns) {
            sum_returns+= ret;
        }
        double mean_returns = sum_returns / returns.size();
        double sum_sq = 0.0;
        for (const double& ret : returns) {
            sum_sq += (ret - mean_returns)*(ret - mean_returns);
        }
        double var_returns = sum_sq / returns.size();
        report.volatility = std::sqrt(var_returns);
    } else {
        report.volatility = 0.0;
    }

    // compute max drawdowns
    double max_drawdown = 0.0;
    double peak = std::numeric_limits<double>::lowest();
    for (size_t i=0; i <equity.size(); i++){
        double current = equity[i].second; 
        if (peak < current) {peak = current;}
        double drd = (peak - current)/peak;
        if (max_drawdown < (drd)) {max_drawdown = drd;}
    }
    report.max_drawdown = max_drawdown;

    // compute number of trades
    report.num_trades = trades.size();
    return report;
}

void display_backtest_report(const BacktestReport& report) {
    std::cout << "========= BACKTEST REPORT ========= \n\n";
    std::cout << "Final equity: " << report.final_equity << std::endl;
    std::cout << "Final return: " <<report.total_return << std::endl;
    std::cout << "Max drawdown: " << report.max_drawdown << std::endl;  
}