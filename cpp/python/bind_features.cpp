#include "bindings.hpp"

#include "data/features/cross_section.hpp"
#include "data/features/config/feature_config.hpp"
#include "data/features/feature_engine.hpp"
#include "data/features/config/feature_kind.hpp"
#include "data/features/feature_schema.hpp"
#include "data/features/feature_types.hpp"

#include <pybind11/stl.h>

#include <utility>

void bind_features(py::module_& m) {
    py::enum_<FeatureKind>(m, "FeatureKind", "Supported online feature id.")
        .value("Return", FeatureKind::Return)
        .value("LogReturn", FeatureKind::LogReturn)
        .value("ReturnVariance", FeatureKind::ReturnVariance)
        .value("ReturnVolatility", FeatureKind::ReturnVolatility)
        .value("PriceZ", FeatureKind::PriceZ)
        .value("VolumeZ", FeatureKind::VolumeZ)
        .value("HighLowRange", FeatureKind::HighLowRange)
        .value("CloseToSma", FeatureKind::CloseToSma)
        .export_values();

    py::class_<FeatureSchema>(m, "FeatureSchema", "Ordered online feature schema.")
        .def(py::init<std::vector<FeatureKind>>(), py::arg("features"))
        .def(py::init<const std::vector<std::string>&>(), py::arg("names"))
        .def("size", &FeatureSchema::size)
        .def("empty", &FeatureSchema::empty)
        .def("names", &FeatureSchema::names, py::return_value_policy::reference_internal)
        .def("features", &FeatureSchema::features, py::return_value_policy::reference_internal)
        .def("name", &FeatureSchema::name, py::arg("idx"), py::return_value_policy::reference_internal)
        .def("feature", &FeatureSchema::feature, py::arg("idx"))
        .def("contains", &FeatureSchema::contains, py::arg("id"))
        .def("index_of", &FeatureSchema::index_of, py::arg("id"));

    py::class_<FeatureConfig>(m, "FeatureConfig", "Online feature engine config.")
        .def(
            py::init([](FeatureSchema schema, std::size_t window, bool enable_cross_section) {
                return FeatureConfig{std::move(schema), window, enable_cross_section};
            }),
            py::arg("schema"),
            py::arg("window"),
            py::arg("enable_cross_section") = false
        )
        .def_readwrite("schema", &FeatureConfig::schema)
        .def_readwrite("window", &FeatureConfig::window)
        .def_readwrite("enable_cross_section", &FeatureConfig::enable_cross_section);

    py::class_<FeatureRow>(m, "FeatureRow")
        .def_readwrite("symbol_id", &FeatureRow::symbol_id)
        .def_readwrite("values", &FeatureRow::values);

    py::class_<CrossSectionResult>(m, "CrossSectionResult")
        .def_readwrite("timestamp", &CrossSectionResult::timestamp)
        .def_readwrite("symbols", &CrossSectionResult::symbols)
        .def_readwrite("raw", &CrossSectionResult::raw)
        .def_readwrite("zscore", &CrossSectionResult::zscore)
        .def_readwrite("rank_pct", &CrossSectionResult::rank_pct);

    py::class_<FeatureView>(m, "FeatureView")
        .def_readonly("timestamp", &FeatureView::timestamp)
        .def("has_row", &FeatureView::has_row)
        .def("has_cross_section", &FeatureView::has_cross_section)
        .def("row_symbol_id", [](const FeatureView& v) -> std::optional<SymbolId> {
            if (!v.row) return std::nullopt;
            return v.row->symbol_id;
        })
        .def("row_values", [](const FeatureView& v) -> std::vector<double> {
            if (!v.row) return {};
            return v.row->values;
        })
        .def("schema_names", [](const FeatureView& v) -> std::vector<std::string> {
            if (!v.schema) return {};
            return v.schema->names();
        });

    py::class_<FeatureEngine>(m, "FeatureEngine")
        .def(
            py::init<std::vector<SymbolId>, FeatureSchema, std::size_t>(),
            py::arg("universe"),
            py::arg("schema"),
            py::arg("window")
        )
        .def("push", &FeatureEngine::push, py::arg("event"))
        .def("ready", &FeatureEngine::ready, py::arg("id"))
        .def("ready_all", &FeatureEngine::ready_all)
        .def("schema", &FeatureEngine::schema, py::return_value_policy::reference_internal)
        .def("universe", &FeatureEngine::universe, py::return_value_policy::reference_internal)
        .def("latest_row", &FeatureEngine::latest_row, py::arg("id"), py::return_value_policy::reference_internal)
        .def("latest_rows", &FeatureEngine::latest_rows)
        .def("compute_cross_section", &FeatureEngine::compute_cross_section, py::arg("timestamp"))
        .def("reset", &FeatureEngine::reset);
}
