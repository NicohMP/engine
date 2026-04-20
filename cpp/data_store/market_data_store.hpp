#pragma once
#include "event_types.hpp"
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>

using SymbolId = std::uint32_t; // numerical stock id type

struct Bar {
    /* MarketEvent alias with numerical id instead of string ticker*/
    int64_t timestamp;
    SymbolId symbol_id;
    double open;
    double high;
    double low;
    double close;
    double volume;
};

struct TimeSlice {
    /* View of all the ticks for one timestamp. 
     Contains just the timestampin question, 
     the index of the fist tick and last tick of the slice inside the dataframe
     */
    int64_t timestamp;
    size_t begin_idx;
    size_t end_idx;
};

struct BuildStats {
    /* Stats of Market data storage process */
    size_t input_rows = 0;
    size_t kept_rows = 0;
    size_t dropped_invalid = 0;
    size_t dropped_duplicates = 0;
    size_t num_symbols = 0;
    size_t num_slices = 0;
};

std::string normalize_ticker(const std::string& raw_ticker);
struct BuildResult;

class MarketDataStore {
    /*Read-only container of clean market data (OHLVC)
    */
    private:
        std::vector<Bar> bars_;
        std::vector<TimeSlice> slices_;
        std::unordered_map<SymbolId, std::string> id_to_symbol_;
        std::unordered_map<std::string, SymbolId> symbol_to_id_;

        
        explicit MarketDataStore(
            std::vector<Bar> bars,
            std::vector<TimeSlice> slices,
            std::unordered_map<SymbolId, std::string> id_to_symbol,
            std::unordered_map<std::string, SymbolId> symbol_to_id
        );
    public:
        static BuildResult from_market_events(std::vector<MarketEvent> rows);
        SymbolId get_or_add_id(const std::string& raw_ticker) ;
        std::string get_symbol(const SymbolId& id) const;
        const std::vector<Bar>& bars() const { return bars_; }
        const std::vector<TimeSlice>& slices() const { return slices_; }

    };

struct BuildResult {
    /*Container for market data and build stats
    (output of MarketDataStore::from_market_event)
    */
    MarketDataStore store;
    BuildStats stats;
};
