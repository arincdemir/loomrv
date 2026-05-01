# Find Boost (ICL is header-only, no components needed)
cmake_policy(SET CMP0167 NEW)
find_package(Boost CONFIG REQUIRED)

add_executable(bench_runner 
    benchmark_engine.cpp
    benchmark_interval_set.cpp
    benchmark_multi_property.cpp
)

# Link the SAME library and SAME Catch2, plus Boost headers
target_link_libraries(bench_runner PRIVATE loomrv_lib Catch2::Catch2WithMain Boost::boost)

# Copy Data: Root/data -> build/benchmarks/data
# We copy it here too, so the benchmark executable can find it locally
add_custom_command(TARGET bench_runner POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data
    $<TARGET_FILE_DIR:bench_runner>/data
)