cmake_minimum_required(VERSION 3.16)
project(LogPulse VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

add_library(logpulse_core
    src/LogParser.cpp
    src/LogReader.cpp
    src/LogAnalyzer.cpp
    src/ThreadPool.cpp
    src/ReportGenerator.cpp
    src/Benchmark.cpp
)
target_include_directories(logpulse_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(LogPulse src/main.cpp)
target_link_libraries(LogPulse PRIVATE logpulse_core)

option(LOGPULSE_BUILD_TESTS "Build LogPulse tests" ON)
if(LOGPULSE_BUILD_TESTS)
    enable_testing()
    add_executable(test_parser tests/test_parser.cpp)
    target_link_libraries(test_parser PRIVATE logpulse_core)
    add_test(NAME parser COMMAND test_parser)

    add_executable(test_analyzer tests/test_analyzer.cpp)
    target_link_libraries(test_analyzer PRIVATE logpulse_core)
    add_test(NAME analyzer COMMAND test_analyzer)

    add_executable(test_threadpool tests/test_threadpool.cpp)
    target_link_libraries(test_threadpool PRIVATE logpulse_core)
    add_test(NAME threadpool COMMAND test_threadpool)

    add_executable(test_integration tests/test_integration.cpp)
    target_link_libraries(test_integration PRIVATE logpulse_core)
    add_test(NAME integration COMMAND test_integration)
endif()
