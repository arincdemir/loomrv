cmake_minimum_required(VERSION 3.15)
project(loomrv VERSION 1.0.0 LANGUAGES CXX)

option(ENABLE_COVERAGE "Enable code coverage" OFF)
set(LOOMRV_TESTDATA_URL
    "https://github.com/arincdemir/loomrv/releases/download/timescales-data-v1/data.zip"
)
set(LOOMRV_TESTDATA_DIR "${CMAKE_SOURCE_DIR}/data/fullsuite")
set(LOOMRV_TESTDATA_ARCHIVE "${CMAKE_BINARY_DIR}/loomrv-testdata.zip")
set(LOOMRV_TESTDATA_SCRIPT "${CMAKE_BINARY_DIR}/download_testdata.cmake")

set(LOOMRV_TESTDATA_SCRIPT_CONTENT [=[
if(EXISTS "@LOOMRV_TESTDATA_DIR@")
    message(STATUS "Test data already present at @LOOMRV_TESTDATA_DIR@")
else()
    file(MAKE_DIRECTORY "@CMAKE_SOURCE_DIR@/data")
    message(STATUS "Downloading test data from @LOOMRV_TESTDATA_URL@")
    file(DOWNLOAD "@LOOMRV_TESTDATA_URL@" "@LOOMRV_TESTDATA_ARCHIVE@" SHOW_PROGRESS STATUS status LOG log)
    list(GET status 0 code)
    list(GET status 1 msg)
    if(NOT code EQUAL 0)
        message(FATAL_ERROR "Failed to download test data: ${msg}\n${log}")
    endif()

    execute_process(
        COMMAND "@CMAKE_COMMAND@" -E tar xvf "@LOOMRV_TESTDATA_ARCHIVE@"
        WORKING_DIRECTORY "@CMAKE_SOURCE_DIR@"
        RESULT_VARIABLE res
    )
    if(NOT res EQUAL 0)
        message(FATAL_ERROR "Failed to extract test data from @LOOMRV_TESTDATA_ARCHIVE@")
    endif()

    file(REMOVE "@LOOMRV_TESTDATA_ARCHIVE@")
endif()
]=])

string(CONFIGURE "${LOOMRV_TESTDATA_SCRIPT_CONTENT}" LOOMRV_TESTDATA_SCRIPT_CONTENT @ONLY)
file(WRITE "${LOOMRV_TESTDATA_SCRIPT}" "${LOOMRV_TESTDATA_SCRIPT_CONTENT}")

add_custom_target(download_testdata
    COMMAND "${CMAKE_COMMAND}" -P "${LOOMRV_TESTDATA_SCRIPT}"
    BYPRODUCTS "${LOOMRV_TESTDATA_DIR}"
    COMMENT "Downloading test data if missing"
)

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
