#include "market_data_store.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>

std::string normalize_ticker(const std::string& raw_ticker){
    /* Normalize ticker string by :
        - removing leading/trailing spaces
        _ uppercase remaining characters
    */
    std::size_t begin = 0;
    while (begin < raw_ticker.size() &&
           std::isspace(static_cast<unsigned char>(raw_ticker[begin]))) {
        ++begin;
    }

    std::size_t end = raw_ticker.size();
    while (end > begin &&
           std::isspace(static_cast<unsigned char>(raw_ticker[end - 1]))) {
        --end;
    }

    std::string ticker = raw_ticker.substr(begin, end - begin);
    for (char& c : ticker) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return ticker;
}

static SymbolId get_add_symbol_build(
    const std::string& raw_ticker,
    std::unordered_map<std::string, SymbolId>& symbol_to_id,
    std::unordered_map<SymbolId, std::string>& id_to_symbol){
        /* Get id of raw ticker if stored, ohterwise add it the map and return it*/
        std::string ticker = normalize_ticker(raw_ticker); // trim + uppercase
        auto it = symbol_to_id.find(ticker);
        if (it != symbol_to_id.end()) return it->second;

        SymbolId id = static_cast<SymbolId>(id_to_symbol.size());
        id_to_symbol.emplace(id, ticker);
        symbol_to_id.emplace(ticker, id);
        return id;
}



SymbolId MarketDataStore::get_or_add_id(const std::string& raw_ticker) {
    /* Get id of raw ticker if stored, ohterwise add it the map and return it*/
    std::string ticker = normalize_ticker(raw_ticker); // trim + uppercase
    auto it = symbol_to_id_.find(ticker);
    if (it != symbol_to_id_.end()) return it->second;

    SymbolId id = static_cast<SymbolId>(id_to_symbol_.size());
    id_to_symbol_.emplace(id, ticker);
    symbol_to_id_.emplace(ticker, id);
    return id;
}

MarketDataStore::MarketDataStore(std::vector<Bar> bars,
            std::vector<TimeSlice> slices,
            std::unordered_map<SymbolId, std::string> id_to_symbol,
            std::unordered_map<std::string, SymbolId> symbol_to_id
        ) : bars_(std::move(bars)),
            slices_(std::move(slices)),
            id_to_symbol_(std::move(id_to_symbol)),
            symbol_to_id_(std::move(symbol_to_id))
        {}


namespace {
bool is_valid_row(const MarketEvent& e) {
    if (e.timestamp <= 0) return false;
    if (!std::isfinite(e.open) || !std::isfinite(e.high) ||
        !std::isfinite(e.low)  || !std::isfinite(e.close) ||
        !std::isfinite(e.volume)) return false;
    if (e.open <= 0.0 || e.high <= 0.0 || e.low <= 0.0 || e.close <= 0.0) return false;
    if (e.volume < 0.0) return false;
    if (e.low > e.open || e.low > e.close || e.high < e.open || e.high < e.close) return false;
    return true;
}
}

std::string MarketDataStore::get_symbol(const SymbolId& id) const {
    auto it = id_to_symbol_.find(id);
    if (it == id_to_symbol_.end()) {
        throw std::runtime_error("No ticker found for symbol id: " + std::to_string(id));
    }
    return it->second;
}


BuildResult MarketDataStore::from_market_events(std::vector<MarketEvent> rows){

    // Declare build stats
    BuildStats stats{};
    stats.input_rows = rows.size();

    // Declare MarketDataStore arguments
    std::unordered_map<SymbolId, std::string> id_to_symbol;
    std::unordered_map<std::string, SymbolId> symbol_to_id;
    std::vector<Bar> bars;
    bars.reserve(rows.size());
    // First pass : build bars and id/symbol maps
    for (const MarketEvent& row : rows){
        if (!is_valid_row(row)) {
            ++stats.dropped_invalid;
            continue;
        }
        std::string ticker = normalize_ticker(row.ticker);
        if (ticker.empty()) {
            ++stats.dropped_invalid;
            continue;
        }
        SymbolId id = get_add_symbol_build(ticker,symbol_to_id, id_to_symbol);
        Bar bar{row.timestamp,id,row.open, row.high,row.low, row.close, row.volume};
        bars.push_back(bar);
    }
    // Sort bars by timestamp
    std::sort(bars.begin(), bars.end(), [](const Bar& a, const Bar& b) {
    if (a.timestamp != b.timestamp) return a.timestamp < b.timestamp;
    return a.symbol_id < b.symbol_id;
    });

    // Dedupe
    if (!bars.empty()) {
        size_t write = 1; // next kept slot
        for (size_t read = 1; read < bars.size(); ++read) {
            const bool same_key =
                bars[read].timestamp == bars[write - 1].timestamp &&
                bars[read].symbol_id == bars[write - 1].symbol_id;

            if (same_key) { // policy last-wins: overwrite previous kept row
                bars[write - 1] = bars[read];
                ++stats.dropped_duplicates;
            } else {
                bars[write] = bars[read];
                ++write;
            }
        }
        bars.resize(write);
    }

    std::vector<TimeSlice> slices;
    size_t start = 0; size_t i = 0;
    while (i < bars.size()){
        while (i < bars.size() && bars[start].timestamp == bars[i].timestamp){
            i++;
        }
        slices.push_back(TimeSlice{bars[start].timestamp, start, i});
        start = i;
    }

    stats.kept_rows = bars.size();
    stats.num_symbols = id_to_symbol.size();
    stats.num_slices = slices.size();

    BuildResult result{
        MarketDataStore(
            std::move(bars),
            std::move(slices),
            std::move(id_to_symbol),
            std::move(symbol_to_id)
        ),
        stats
    };
    return result;
}

