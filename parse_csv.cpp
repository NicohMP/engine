#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <set>
#include "parse_csv.hpp"
using std::string;
using std::ifstream;

int64_t timestamp_to_int64(const string& timestamp) {
    string clean = timestamp;
    clean.erase(
        std::remove(clean.begin(), clean.end(), '-'),
        clean.end()
    );
    return std::stoll(clean);
}

Kline parse_kline(string line) {
    std::stringstream ss(line); // string stream going through the line 
    string cell; // holder for string values in a line
    Kline k; // declare Kline to store line values
    std::getline(ss, cell, ','); // parse open timestamps as string
    k.open_time_us = std::stoll(cell); // convert to int64/long long

    
    std::getline(ss, cell, ',');
    k.open = std::stod(cell); // parse open and convert to double

    std::getline(ss, cell, ',');
    k.high = std::stod(cell); // parse high and convert to double

    std::getline(ss, cell, ','); // parse low and convert to double
    k.low = std::stod(cell);
    
    std::getline(ss, cell, ','); // parse close and convert to double
    k.close = std::stod(cell);

    std::getline(ss, cell, ','); // parse volume base and convert to double
    k.volume_base = std::stod(cell);

    std::getline(ss, cell, ','); 
    k.close_time_us = std::stoll(cell);
    
    std::getline(ss, cell, ','); 
    k.quote_volume = std::stod(cell);

    std::getline(ss, cell, ','); 
    k.trade_count = std::stoi(cell);

    std::getline(ss, cell, ','); 
    k.taker_buy_base = std::stod(cell);

    std::getline(ss, cell, ','); 
    k.taker_buy_quote = std::stod(cell);

    std::getline(ss, cell, '\n'); // skip useles "ignore" column

    return k;
}

Tick parse_line_into_tick(string line) {
    std::stringstream ss(line); // string stream goign through the line 
    string cell; // holder for string values in a line
    Tick t; // declare Tick to store line values
    std::getline(ss, cell, ','); // parse timestamps as a string
    t.timestamp = timestamp_to_int64(cell);

    std::getline(ss, t.ticker, ','); // parse ticker already a string 
    
    std::getline(ss, cell, ',');
    t.open = std::stod(cell); // parse open and convert to double

    std::getline(ss, cell, ',');
    t.high = std::stod(cell); // parse high and convert to double

    std::getline(ss, cell, ','); // parse low and convert to double
    t.low = std::stod(cell);

    std::getline(ss, cell, ','); // parse volume and convert to double
    t.volume = std::stod(cell);

    std::getline(ss, cell, '\n'); // parse close and convert to double
    t.close = std::stod(cell);

    return t;
}

Tick kline_to_tick(const Kline& kline, const string& ticker) {
    Tick t;
    t.ticker = ticker;
    t.timestamp = kline.open_time_us;
    t.open = kline.open;
    t.high = kline.high;
    t.low = kline.low;
    t.volume = kline.volume_base;
    t.close = kline.close;
    return t;
}

bool tick_checks(Tick tick) {
    // check attributes aren't empty
    if (tick.timestamp <= 0) {return false;}
    if (tick.ticker.empty()) {return false;}
    // check numerical values are positive
    double min_double = std::min({tick.open, tick.close, tick.low, tick.high});
    if (min_double < 0) {return false;}
    if (tick.volume < 0) {return false;}
    if (!(tick.low <= tick.open) || !(tick.open <= tick.high)) {return false;}
    if (!(tick.low <= tick.close) || !(tick.close <= tick.high)) {return false;}
    return true;
}

void report_skipped_lines(std::vector<int> skipped_lines) {
    if (!(skipped_lines.empty())) {
        std::cout << "Lines ";
        for (int index : skipped_lines) {
            std::cout << index << ", ";
        }
        std::cout << "were skipped because incorrect.\n";
    }
}

std::ifstream open_file(string filename) {
    std::ifstream file(filename); 
    if (!file) {
    throw std::runtime_error("Could not open file: " + filename);
    } else {
        return file;
    }
}

std::vector<Tick> parse_csv(string filepath) {
    std::vector <Tick> all_ticks;
    std::string line;
    std::ifstream file = open_file(filepath);

    // First line should be the CSV header. If missing, file is empty.
    if (!std::getline(file, line)) {
        std::cerr << "Empty CSV file: " << filepath << "\n";
        return all_ticks;
    }

    int line_no = 1;
    std::vector<int> skipped_lines;
    std::cout << "Parsing file... \n";
    while (std::getline(file, line)) {
        line_no++;
        try {
            Tick t = parse_line_into_tick(line);
            if (!tick_checks(t)) {
                std::cerr << "Line " << line_no << ": failed tick_checks\n";
                skipped_lines.push_back(line_no);
                continue;
            }
            all_ticks.push_back(t);
        } catch (const std::exception& e) {
            std::cerr << "Line " << line_no << ": parse error (" << e.what() << ")\n";
            skipped_lines.push_back(line_no);
            continue;
        }
    }

    std::cout << "Parsed " << line_no << " lines.\n";
    report_skipped_lines(skipped_lines);
    return all_ticks;

    // std::set<string>  tickers;
    // for (const Tick& t : all_ticks) {
    //     tickers.insert(t.ticker);
    // }
    // for (const string& ticker : tickers) {
    //     std::cout << ticker << std::endl;
    // }

}

std::vector<Tick> parse_kline_csv(string filepath, string ticker) {
    std::vector <Tick> all_ticks;
    std::string line;
    std::ifstream file = open_file(filepath);

    // First line should be the CSV header. If missing, file is empty.
    if (!std::getline(file, line)) {
        std::cerr << "Empty CSV file: " << filepath << "\n";
        return all_ticks;
    }

    int line_no = 1;
    std::vector<int> skipped_lines;
    std::cout << "Parsing file... \n";
    while (std::getline(file, line)) {
        line_no++;
        try {
            Kline kline = parse_kline(line);
            Tick t = kline_to_tick(kline, ticker);
            if (!tick_checks(t)) {
                std::cerr << "Line " << line_no << ": failed tick_checks\n";
                skipped_lines.push_back(line_no);
                continue;
            }
            all_ticks.push_back(t);
        } catch (const std::exception& e) {
            std::cerr << "Line " << line_no << ": parse error (" << e.what() << ")\n";
            skipped_lines.push_back(line_no);
            continue;
        }
    }

    std::cout << "Parsed " << line_no << " lines.\n";
    report_skipped_lines(skipped_lines);
    return all_ticks;

    // std::set<string>  tickers;
    // for (const Tick& t : all_ticks) {
    //     tickers.insert(t.ticker);
    // }
    // for (const string& ticker : tickers) {
    //     std::cout << ticker << std::endl;
    // }

}