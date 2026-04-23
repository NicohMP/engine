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
    data/parse_csv.cpp
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
    data_store/market_data_store.cpp
    simulation/simulator.cpp
    strategy/rolling_moments.cpp

)

# Tell the `engine` executable where to look for project header files.
# PRIVATE means these include paths are only needed while compiling this target.
target_include_directories(engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/app
    ${CMAKE_CURRENT_SOURCE_DIR}/backtest
    ${CMAKE_CURRENT_SOURCE_DIR}/data
    ${CMAKE_CURRENT_SOURCE_DIR}/execution
    ${CMAKE_CURRENT_SOURCE_DIR}/portfolio
    ${CMAKE_CURRENT_SOURCE_DIR}/strategy
    ${CMAKE_CURRENT_SOURCE_DIR}/risk
    ${CMAKE_CURRENT_SOURCE_DIR}/data_store
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
    data/parse_csv.cpp
    portfolio/portfolio.cpp
    portfolio/position.cpp
    core/event_types.cpp
    strategy/base.cpp
    strategy/random.cpp
    backtest/backtester.cpp
    execution/execution_engine.cpp
    risk/simple.cpp
    data_store/market_data_store.cpp
    core/engine_core.cpp

)

# Tell the `engine_cpp` Python module where to find project header files.
target_include_directories(engine_cpp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/app
    ${CMAKE_CURRENT_SOURCE_DIR}/backtest
    ${CMAKE_CURRENT_SOURCE_DIR}/data
    ${CMAKE_CURRENT_SOURCE_DIR}/execution
    ${CMAKE_CURRENT_SOURCE_DIR}/portfolio
    ${CMAKE_CURRENT_SOURCE_DIR}/strategy
    ${CMAKE_CURRENT_SOURCE_DIR}/risk
    ${CMAKE_CURRENT_SOURCE_DIR}/data_store
    ${CMAKE_CURRENT_SOURCE_DIR}/core
)

add_custom_command(TARGET engine_cpp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/python/engine_cpp.pyi
        $<TARGET_FILE_DIR:engine_cpp>/engine_cpp.pyi
)

add_executable(test_data_store
    tests/test_data_store.cpp
    data_store/market_data_store.cpp
)

target_include_directories(test_data_store PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/data_store
    ${CMAKE_CURRENT_SOURCE_DIR}/backtest
    ${CMAKE_CURRENT_SOURCE_DIR}/data
)

add_test(NAME test_data_store COMMAND test_data_store)

add_executable(test_rolling_moments
    tests/test_rolling_moments.cpp
    strategy/rolling_moments.cpp
)

target_include_directories(test_rolling_moments PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/strategy
)

add_test(NAME test_rolling_moments COMMAND test_rolling_moments)
