#pragma once

#include <cstdint>
#include <string>

struct Tick {
    int64_t timestamp;
    std::string ticker;
    double open;
    double high;
    double low;
    double volume;
    double close;
};
