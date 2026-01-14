cmake_minimum_required(VERSION 3.16)
project(sd_e2_benchmark_and_optimize LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT MSVC)
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Prefer a system install if available (helps in offline environments)
find_package(benchmark QUIET)

if(NOT benchmark_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
  )
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
  set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(benchmark)
endif()

add_executable(bench_layout
  src/bench_layout.cpp
)
target_include_directories(bench_layout PRIVATE include)
target_link_libraries(bench_layout PRIVATE benchmark::benchmark)

