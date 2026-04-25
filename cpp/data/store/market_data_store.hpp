#pragma once
#include "core/event_types.hpp"
#include "tick.hpp"
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>


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
        std::vector<MarketEvent> events_;
        std::vector<TimeSlice> slices_;
        std::unordered_map<SymbolId, std::string> id_to_symbol_;
        std::unordered_map<std::string, SymbolId> symbol_to_id_;

        
        explicit MarketDataStore(
            std::vector<MarketEvent> events,
            std::vector<TimeSlice> slices,
            std::unordered_map<SymbolId, std::string> id_to_symbol,
            std::unordered_map<std::string, SymbolId> symbol_to_id
        );
    public:
        static BuildResult from_ticks(std::vector<Tick> rows);
        SymbolId get_or_add_id(const std::string& raw_ticker) ;
        std::string get_symbol(const SymbolId& id) const;
        SymbolId get_id(const std::string& raw_ticker) const;
        const std::vector<MarketEvent>& bars() const { return events_; }
        const std::vector<TimeSlice>& slices() const { return slices_; }

    };

struct BuildResult {
    /*Container for market data and build stats
    (output of MarketDataStore::from_market_event)
    */
    MarketDataStore store;
    BuildStats stats;
};
