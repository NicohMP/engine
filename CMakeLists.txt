cmake_minimum_required(VERSION 3.10)

project(engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(engine
    main.cpp
    analytics.cpp
    parse_csv.cpp
    event_types.cpp
    trading.cpp
    trading_types.cpp
    backtester.cpp
    portfolio.cpp
    execution_engine.cpp
    strategies/sma_strategy.cpp
)

target_include_directories(engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
