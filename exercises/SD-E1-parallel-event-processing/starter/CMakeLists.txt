cmake_minimum_required(VERSION 3.16)
project(sd_e1_parallel_event_processing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_OPENMP "Enable OpenMP parallelism" OFF)

# Warnings
if(NOT MSVC)
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# OpenMP
if(ENABLE_OPENMP)
  find_package(OpenMP REQUIRED)
endif()

add_library(sd_e1_core
  src/EventProcessor.cpp
)
target_include_directories(sd_e1_core PUBLIC include)

if(ENABLE_OPENMP)
  target_link_libraries(sd_e1_core PUBLIC OpenMP::OpenMP_CXX)
  target_compile_definitions(sd_e1_core PUBLIC CSC2026_USE_OPENMP=1)
endif()

add_executable(event_processor
  src/main.cpp
)
target_link_libraries(event_processor PRIVATE sd_e1_core)

include(CTest)

if(BUILD_TESTING)
  # Prefer a system install if available (helps in offline environments)
  find_package(Catch2 3 QUIET)

  if(NOT Catch2_FOUND)
    include(FetchContent)
    FetchContent_Declare(
      Catch2
      GIT_REPOSITORY https://github.com/catchorg/Catch2.git
      GIT_TAG v3.5.3
    )
    FetchContent_MakeAvailable(Catch2)
  endif()

  add_executable(test_sd_e1
    tests/test_event_processor.cpp
  )
  target_link_libraries(test_sd_e1 PRIVATE sd_e1_core Catch2::Catch2WithMain)
  add_test(NAME sd_e1_tests COMMAND test_sd_e1)

  # Make races reproducible: ensure we run with >1 thread under ctest
  if(ENABLE_OPENMP)
    set_tests_properties(sd_e1_tests PROPERTIES ENVIRONMENT "OMP_NUM_THREADS=4")
  endif()
endif()

