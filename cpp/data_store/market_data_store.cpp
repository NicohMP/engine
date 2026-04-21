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

MarketDataStore::MarketDataStore(std::vector<MarketEvent> events,
            std::vector<TimeSlice> slices,
            std::unordered_map<SymbolId, std::string> id_to_symbol,
            std::unordered_map<std::string, SymbolId> symbol_to_id
        ) : events_(std::move(events)),
            slices_(std::move(slices)),
            id_to_symbol_(std::move(id_to_symbol)),
            symbol_to_id_(std::move(symbol_to_id))
        {}


namespace {
bool is_valid_row(const Tick& t) {
    if (t.timestamp <= 0) return false;
    if (!std::isfinite(t.open) || !std::isfinite(t.high) ||
        !std::isfinite(t.low)  || !std::isfinite(t.close) ||
        !std::isfinite(t.volume)) return false;
    if (t.open <= 0.0 || t.high <= 0.0 || t.low <= 0.0 || t.close <= 0.0) return false;
    if (t.volume < 0.0) return false;
    if (t.low > t.open || t.low > t.close || t.high < t.open || t.high < t.close) return false;
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


SymbolId MarketDataStore::get_id(const std::string& ticker) const {
    std::string clean_t = normalize_ticker(ticker);
    auto it = symbol_to_id_.find(clean_t);
    if (it == symbol_to_id_.end()) {
        throw std::runtime_error("No id found for ticker: " + clean_t);
    }
    return it->second;
}


BuildResult MarketDataStore::from_ticks(std::vector<Tick> rows){

    // Declare build stats
    BuildStats stats{};
    stats.input_rows = rows.size();

    // Declare MarketDataStore arguments
    std::unordered_map<SymbolId, std::string> id_to_symbol;
    std::unordered_map<std::string, SymbolId> symbol_to_id;
    std::vector<MarketEvent> events;
    events.reserve(rows.size());
    // First pass : build events and id/symbol maps
    for (const Tick& row : rows){
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
        MarketEvent event{row.timestamp,id,ticker, row.open, row.high,row.low, row.close, row.volume};
        events.push_back(event);
    }
    // Sort events by timestamp
    std::sort(events.begin(), events.end(), [](const MarketEvent& a, const MarketEvent& b) {
    if (a.timestamp != b.timestamp) return a.timestamp < b.timestamp;
    return a.symbol_id < b.symbol_id;
    });

    // Dedupe
    if (!events.empty()) {
        size_t write = 1; // next kept slot
        for (size_t read = 1; read < events.size(); ++read) {
            const bool same_key =
                events[read].timestamp == events[write - 1].timestamp &&
                events[read].symbol_id == events[write - 1].symbol_id;

            if (same_key) { // policy last-wins: overwrite previous kept row
                events[write - 1] = events[read];
                ++stats.dropped_duplicates;
            } else {
                events[write] = events[read];
                ++write;
            }
        }
        events.resize(write);
    }

    std::vector<TimeSlice> slices;
    size_t start = 0; size_t i = 0;
    while (i < events.size()){
        while (i < events.size() && events[start].timestamp == events[i].timestamp){
            i++;
        }
        slices.push_back(TimeSlice{events[start].timestamp, start, i});
        start = i;
    }

    stats.kept_rows = events.size();
    stats.num_symbols = id_to_symbol.size();
    stats.num_slices = slices.size();

    BuildResult result{
        MarketDataStore(
            std::move(events),
            std::move(slices),
            std::move(id_to_symbol),
            std::move(symbol_to_id)
        ),
        stats
    };
    return result;
}

