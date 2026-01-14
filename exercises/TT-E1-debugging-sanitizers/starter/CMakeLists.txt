cmake_minimum_required(VERSION 3.16)
project(tt_e1_debugging_sanitizers LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_SANITIZERS "Enable ASan/UBSan for debugging" OFF)

# -----------------------------------------------------------------------------
# Sanitizers (course expects Clang for consistent ASan output)
# -----------------------------------------------------------------------------
if(ENABLE_SANITIZERS)
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR
      "ENABLE_SANITIZERS is supported in this course using Clang.\n"
      "Reconfigure with:\n"
      "  rm -rf build-asan\n"
      "  CC=clang CXX=clang++ cmake -B build-asan -G Ninja -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON\n"
      "  cmake --build build-asan -j\"$(nproc)\"\n"
      "  ./build-asan/analyze\n"
    )
  endif()

  message(STATUS "Sanitizers: ON (address,undefined)")

  add_library(tt_e1_sanitize_flags INTERFACE)

  # Critical: disable libstdc++ bounds-check abort so ASan can report (slides 15/16 style)
  target_compile_definitions(tt_e1_sanitize_flags INTERFACE
    _GLIBCXX_NO_ASSERTIONS
  )

  # Safety: if something injects these, remove them
  target_compile_options(tt_e1_sanitize_flags INTERFACE
    -U_GLIBCXX_ASSERTIONS
    -U_GLIBCXX_DEBUG
    -U_GLIBCXX_DEBUG_PEDANTIC

    -fsanitize=address,undefined
    -fno-omit-frame-pointer
    -g
  )

  target_link_options(tt_e1_sanitize_flags INTERFACE
    -fsanitize=address,undefined
  )
endif()

# -----------------------------------------------------------------------------
# Core library for the exercise
# -----------------------------------------------------------------------------
add_library(tt_e1_core
  src/TrackReconstructor.cpp
)
target_include_directories(tt_e1_core PUBLIC include)

if(ENABLE_SANITIZERS)
  target_link_libraries(tt_e1_core PUBLIC tt_e1_sanitize_flags)
endif()

# -----------------------------------------------------------------------------
# "Analyze" executable that triggers the bugs
# -----------------------------------------------------------------------------
add_executable(analyze
  src/analyze.cpp
)
target_link_libraries(analyze PRIVATE tt_e1_core)

# -----------------------------------------------------------------------------
# Tests (Catch2)
# -----------------------------------------------------------------------------
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

  add_executable(test_tt_e1
    tests/test_track_reconstructor.cpp
  )
  target_link_libraries(test_tt_e1 PRIVATE tt_e1_core Catch2::Catch2WithMain)

  add_test(NAME tt_e1_tests COMMAND test_tt_e1)
endif()
