// parse_csv.hpp
#pragma once

#include <fstream>
#include <string>
#include <vector>
#include "tick.hpp"


struct Kline {
    int64_t open_time_us;         // col 0
    double open;                    // col 1
    double high;                    // col 2
    double low;                     // col 3
    double close;                   // col 4
    double volume_base;             // col 5
    int64_t close_time_us;        // col 6
    double quote_volume;            // col 7
    int    trade_count;             // col 8
    double taker_buy_base;          // col 9
    double taker_buy_quote;         // col 10
}; 

Kline parse_kline(std::string line);
Tick kline_to_tick(const Kline& kline, const std::string& ticker);

Tick parse_line_into_tick(std::string line);
bool tick_checks(Tick tick);
void report_skipped_lines(std::vector<int> skipped_lines);
std::ifstream open_file(std::string filename);
std::vector<Tick> parse_csv(std::string filepath);
std::vector<Tick> parse_kline_csv(std::string filepath, std::string ticker);

