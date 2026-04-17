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

# Build the normal command-line executable called `engine`.
add_executable(engine
    app/main.cpp
    backtest/backtester.cpp
    backtest/event_types.cpp
    data/parse_csv.cpp
    execution/execution_engine.cpp
    portfolio/portfolio.cpp
    portfolio/position.cpp
    strategy/analytics.cpp
    strategy/base.cpp
    strategy/sma.cpp
    strategy/buy_and_hold.cpp
    strategy/random.cpp
    risk/risk_manager.cpp
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
)

# Tell pybind11 to use CMake's modern Python discovery logic.
set(PYBIND11_FINDPYTHON ON)

# Find the pybind11 CMake package installed in the active Python environment.
find_package(pybind11 CONFIG REQUIRED)

# Build a Python-importable extension module named `engine_cpp`.
# Python will import this with: `import engine_cpp`.
pybind11_add_module(engine_cpp
    python/bindings.cpp
    portfolio/portfolio.cpp
    portfolio/position.cpp
    backtest/event_types.cpp
    strategy/base.cpp
    strategy/random.cpp
    backtest/backtester.cpp
    execution/execution_engine.cpp
    risk/risk_manager.cpp
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
)
