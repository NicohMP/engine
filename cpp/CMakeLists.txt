# Require at least this CMake version before reading the rest of the file.
cmake_minimum_required(VERSION 3.10)

# Name the project and say that it builds C++ code.
project(engine LANGUAGES CXX)

# Compile all targets in this directory as C++17.
set(CMAKE_CXX_STANDARD 17)

# Fail configuration if the compiler cannot provide C++17.
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use standard C++17 instead of compiler-specific extensions like gnu++17.
set(CMAKE_CXX_EXTENSIONS OFF)
enable_testing()

# Build the normal command-line executable called `engine`.
add_executable(engine
    app/main.cpp
    data/extract/parse_csv.cpp
    backtest/backtester.cpp
    core/event_types.cpp
    core/engine_core.cpp
    execution/execution_engine.cpp
    portfolio/portfolio.cpp
    portfolio/position.cpp
    strategy/analytics.cpp
    strategy/base.cpp
    strategy/sma.cpp
    strategy/buy_and_hold.cpp
    strategy/random.cpp
    risk/simple.cpp
    data/store/market_data_store.cpp
    simulation/simulator.cpp
    data/features/rolling_moments.cpp
    data/features/online_feature_state.cpp
    data/features/cross_section.cpp
    data/features/feature_schema.hpp
    data/features/feature_schema.cpp
    data/features/feature_engine.cpp
    data/features/config/feature_kind.hpp
    data/features/config/feature_kind.cpp
    data/features/calculators/moving_average.cpp
    data/features/calculators/volatility.cpp
    data/features/calculators/volatility.hpp
    data/features/calculators/returns.cpp
    data/features/calculators/log_returns.cpp
    data/features/calculators/log_returns.hpp
    data/features/calculators/close_to_sma.cpp
    data/features/calculators/price_z.cpp
    data/features/calculators/volume_z.cpp
    data/features/calculators/high_low_range.cpp
    data/features/factory/feature_factory.cpp
)

# Tell the `engine` executable where to look for project header files.
# PRIVATE means these include paths are only needed while compiling this target.
target_include_directories(engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/app
    ${CMAKE_CURRENT_SOURCE_DIR}/backtest
    ${CMAKE_CURRENT_SOURCE_DIR}/data
    ${CMAKE_CURRENT_SOURCE_DIR}/data/extract
    ${CMAKE_CURRENT_SOURCE_DIR}/data/store
    ${CMAKE_CURRENT_SOURCE_DIR}/data/features
    ${CMAKE_CURRENT_SOURCE_DIR}/execution
    ${CMAKE_CURRENT_SOURCE_DIR}/portfolio
    ${CMAKE_CURRENT_SOURCE_DIR}/strategy
    ${CMAKE_CURRENT_SOURCE_DIR}/risk
    ${CMAKE_CURRENT_SOURCE_DIR}/core
)

# Tell pybind11 to use CMake's modern Python discovery logic.
set(PYBIND11_FINDPYTHON ON)

# Find the pybind11 CMake package installed in the active Python environment.
find_package(pybind11 CONFIG REQUIRED)

# Build a Python-importable extension module named `engine_cpp`.
# Python will import this with: `import engine_cpp`.
pybind11_add_module(engine_cpp
    python/bindings.cpp
    python/bindings.hpp
    python/bind_core.cpp
    python/bind_data.cpp
    python/bind_features.cpp
    python/bind_portfolio.cpp
    python/bind_execution.cpp
    python/bind_strategy.cpp
    python/bind_risk.cpp
    python/bind_backtest.cpp
    data/extract/parse_csv.cpp
    portfolio/portfolio.cpp
    portfolio/position.cpp
    core/event_types.cpp
    strategy/base.cpp
    strategy/random.cpp
    strategy/buy_and_hold.cpp
    backtest/backtester.cpp
    simulation/simulator.cpp
    execution/execution_engine.cpp
    risk/simple.cpp
    data/store/market_data_store.cpp
    core/engine_core.cpp
    data/features/rolling_moments.cpp
    data/features/online_feature_state.cpp
    data/features/cross_section.cpp
    data/features/feature_schema.cpp
    data/features/feature_engine.cpp
    data/features/config/feature_kind.cpp
    data/features/calculators/moving_average.cpp
    data/features/calculators/volatility.cpp
    data/features/calculators/returns.cpp
    data/features/calculators/log_returns.cpp
    data/features/calculators/close_to_sma.cpp
    data/features/calculators/price_z.cpp
    data/features/calculators/volume_z.cpp
    data/features/calculators/high_low_range.cpp
    data/features/factory/feature_factory.cpp
    python/bind_simulator.cpp
    signal/sma.hpp

)

# Tell the `engine_cpp` Python module where to find project header files.
target_include_directories(engine_cpp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/app
    ${CMAKE_CURRENT_SOURCE_DIR}/backtest
    ${CMAKE_CURRENT_SOURCE_DIR}/data
    ${CMAKE_CURRENT_SOURCE_DIR}/data/extract
    ${CMAKE_CURRENT_SOURCE_DIR}/data/store
    ${CMAKE_CURRENT_SOURCE_DIR}/data/features
    ${CMAKE_CURRENT_SOURCE_DIR}/execution
    ${CMAKE_CURRENT_SOURCE_DIR}/portfolio
    ${CMAKE_CURRENT_SOURCE_DIR}/strategy
    ${CMAKE_CURRENT_SOURCE_DIR}/risk
    ${CMAKE_CURRENT_SOURCE_DIR}/core
)

add_custom_command(TARGET engine_cpp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/python/engine_cpp.pyi
        $<TARGET_FILE_DIR:engine_cpp>/engine_cpp.pyi
)

add_executable(test_data_store
    tests/test_data_store.cpp
    data/store/market_data_store.cpp
)

target_include_directories(test_data_store PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/data/store
    ${CMAKE_CURRENT_SOURCE_DIR}/data/extract
    ${CMAKE_CURRENT_SOURCE_DIR}/backtest
    ${CMAKE_CURRENT_SOURCE_DIR}/data
)

add_test(NAME test_data_store COMMAND test_data_store)

add_executable(test_rolling_moments
    tests/test_rolling_moments.cpp
    data/features/rolling_moments.cpp
)

target_include_directories(test_rolling_moments PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/data/features
)

add_test(NAME test_rolling_moments COMMAND test_rolling_moments)

add_executable(test_online_feature_state
    tests/test_online_feature_state.cpp
    data/features/online_feature_state.cpp
    data/features/rolling_moments.cpp
    data/features/feature_schema.cpp
    data/features/config/feature_kind.cpp
)

target_include_directories(test_online_feature_state PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/data/features
    ${CMAKE_CURRENT_SOURCE_DIR}/data/extract
    ${CMAKE_CURRENT_SOURCE_DIR}/data/store
    ${CMAKE_CURRENT_SOURCE_DIR}/strategy
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/data
)

add_test(NAME test_online_feature_state COMMAND test_online_feature_state)

add_executable(test_feature_engine
    tests/test_feature_engine.cpp
    data/features/feature_engine.cpp
    data/features/online_feature_state.cpp
    data/features/rolling_moments.cpp
    data/features/feature_schema.cpp
    data/features/config/feature_kind.cpp
    data/features/cross_section.cpp
)

target_include_directories(test_feature_engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/data/features
    ${CMAKE_CURRENT_SOURCE_DIR}/data/extract
    ${CMAKE_CURRENT_SOURCE_DIR}/data/store
    ${CMAKE_CURRENT_SOURCE_DIR}/strategy
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/data
)

add_test(NAME test_feature_engine COMMAND test_feature_engine)

add_executable(test_cross_section
    tests/test_cross_section.cpp
    data/features/cross_section.cpp
    data/features/online_feature_state.cpp
    data/features/rolling_moments.cpp
    data/features/feature_schema.cpp
    data/features/config/feature_kind.cpp
)

target_include_directories(test_cross_section PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/data/features
    ${CMAKE_CURRENT_SOURCE_DIR}/data/extract
    ${CMAKE_CURRENT_SOURCE_DIR}/data/store
    ${CMAKE_CURRENT_SOURCE_DIR}/strategy
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/data
)

add_test(NAME test_cross_section COMMAND test_cross_section)

add_executable(test_engine_core_slice
    tests/test_engine_core_slice.cpp
    core/event_types.cpp
    core/engine_core.cpp
    execution/execution_engine.cpp
    portfolio/portfolio.cpp
    portfolio/position.cpp
    strategy/base.cpp
    risk/simple.cpp
    data/features/feature_engine.cpp
    data/features/online_feature_state.cpp
    data/features/rolling_moments.cpp
    data/features/cross_section.cpp
    data/features/feature_schema.cpp
    data/features/config/feature_kind.cpp
)

target_include_directories(test_engine_core_slice PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/execution
    ${CMAKE_CURRENT_SOURCE_DIR}/portfolio
    ${CMAKE_CURRENT_SOURCE_DIR}/strategy
    ${CMAKE_CURRENT_SOURCE_DIR}/risk
    ${CMAKE_CURRENT_SOURCE_DIR}/data
    ${CMAKE_CURRENT_SOURCE_DIR}/data/extract
    ${CMAKE_CURRENT_SOURCE_DIR}/data/features
)

add_test(NAME test_engine_core_slice COMMAND test_engine_core_slice)
