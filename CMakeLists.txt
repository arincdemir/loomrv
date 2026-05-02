cmake_minimum_required(VERSION 3.15)
project(loomrv VERSION 1.0.0 LANGUAGES CXX)

option(ENABLE_COVERAGE "Enable code coverage" OFF)

if(ENABLE_COVERAGE)
    # Add flags for GCC/Clang
    add_compile_options(--coverage)
    add_link_options(--coverage)
endif()

# Standard Setup
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include(FetchContent)

FetchContent_Declare(
    simdjson
    GIT_REPOSITORY https://github.com/simdjson/simdjson.git
    GIT_TAG        v4.6.3 
)

FetchContent_MakeAvailable(simdjson)

FetchContent_Declare(
  cpp-peglib
  GIT_REPOSITORY https://github.com/yhirose/cpp-peglib.git
  GIT_TAG        v1.10.2
)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(cpp-peglib)

# Define the library with source files
add_library(loomrv_lib STATIC
    src/interval_set.cpp
    src/MTLEngine.cpp
    src/binary_row_reader.cpp
    src/json_reader.cpp
    src/json_feeder.cpp
    src/binary_feeder.cpp
)

# Tell CMake where the headers are
target_include_directories(loomrv_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(loomrv_lib PUBLIC peglib simdjson)

# Fetch Catch2 (ONCE for the whole project)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.14.0
)
FetchContent_MakeAvailable(Catch2)



add_executable(loomrv 
    src/main.cpp
)

target_link_libraries(loomrv PRIVATE loomrv_lib)



add_executable(loomrv-perf 
    src/main_perf.cpp
)

target_compile_options(loomrv-perf PRIVATE -g -fno-omit-frame-pointer)
target_link_libraries(loomrv-perf PRIVATE loomrv_lib)

add_executable(test-parse
    src/test_parse.cpp
)
target_compile_options(test-parse PRIVATE -g -O0)
target_link_libraries(test-parse PRIVATE loomrv_lib)

add_executable(check-grammar
    src/check_grammar.cpp
)
target_link_libraries(check-grammar PRIVATE loomrv_lib)

add_executable(count-nodes
    src/count_nodes.cpp
)
target_link_libraries(count-nodes PRIVATE loomrv_lib)

add_executable(test-json src/json_read_main.cpp)
target_link_libraries(test-json PRIVATE simdjson)


# Add Subdirectories
enable_testing()
add_subdirectory(tests)
add_subdirectory(benchmarks)
