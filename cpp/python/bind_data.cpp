#include "bindings.hpp"

#include "data/extract/parse_csv.hpp"
#include "data/extract/tick.hpp"
#include "data/store/market_data_store.hpp"

#include <pybind11/stl.h>

void bind_data(py::module_& m) {
    py::class_<Tick>(m, "Tick", "Raw OHLCV input row.")
        .def(
            py::init<int64_t, std::string, double, double, double, double, double>(),
            py::arg("timestamp"),
            py::arg("ticker"),
            py::arg("open"),
            py::arg("high"),
            py::arg("low"),
            py::arg("volume"),
            py::arg("close")
        )
        .def_readwrite("timestamp", &Tick::timestamp)
        .def_readwrite("ticker", &Tick::ticker)
        .def_readwrite("open", &Tick::open)
        .def_readwrite("high", &Tick::high)
        .def_readwrite("low", &Tick::low)
        .def_readwrite("volume", &Tick::volume)
        .def_readwrite("close", &Tick::close);

    py::class_<TimeSlice>(m, "TimeSlice", "Contiguous range of bars with same timestamp.")
        .def_readonly("timestamp", &TimeSlice::timestamp)
        .def_readonly("begin_idx", &TimeSlice::begin_idx)
        .def_readonly("end_idx", &TimeSlice::end_idx);

    py::class_<BuildStats>(m, "BuildStats", "Stats from MarketDataStore build.")
        .def(py::init<>())
        .def_readwrite("input_rows", &BuildStats::input_rows)
        .def_readwrite("kept_rows", &BuildStats::kept_rows)
        .def_readwrite("dropped_invalid", &BuildStats::dropped_invalid)
        .def_readwrite("dropped_duplicates", &BuildStats::dropped_duplicates)
        .def_readwrite("num_symbols", &BuildStats::num_symbols)
        .def_readwrite("num_slices", &BuildStats::num_slices);

    py::class_<MarketDataStore>(m, "MarketDataStore", "Read-only clean market data store.")
        .def_static("from_ticks", &MarketDataStore::from_ticks, py::arg("rows"))
        .def("get_or_add_id", &MarketDataStore::get_or_add_id, py::arg("ticker"))
        .def("get_symbol", &MarketDataStore::get_symbol, py::arg("symbol_id"))
        .def("get_id", &MarketDataStore::get_id, py::arg("ticker"))
        .def("symbol_ids", &MarketDataStore::symbol_ids)
        .def(
            "bars",
            &MarketDataStore::bars,
            py::return_value_policy::reference_internal
        )
        .def(
            "slices",
            &MarketDataStore::slices,
            py::return_value_policy::reference_internal
        );

    py::class_<BuildResult>(m, "BuildResult", "Build output with store and stats.")
        .def_readwrite("store", &BuildResult::store)
        .def_readwrite("stats", &BuildResult::stats);

    m.def("parse_csv", &parse_csv, py::arg("path"));
}
