cmake_minimum_required(VERSION 3.10)

project(engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
    strategy/sma_strategy.cpp
)

target_include_directories(engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/app
    ${CMAKE_CURRENT_SOURCE_DIR}/backtest
    ${CMAKE_CURRENT_SOURCE_DIR}/data
    ${CMAKE_CURRENT_SOURCE_DIR}/execution
    ${CMAKE_CURRENT_SOURCE_DIR}/portfolio
    ${CMAKE_CURRENT_SOURCE_DIR}/strategy
)
