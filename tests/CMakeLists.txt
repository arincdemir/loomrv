if(ENABLE_COVERAGE)
    find_program(GCOVR_PATH gcovr)

    if(GCOVR_PATH)
        add_custom_target(coverage
            # 1. Run the tests
            COMMAND ${CMAKE_CURRENT_BINARY_DIR}/unit_tests
            
            # 2. Make directory
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
            
            # 3. Generate Report
            COMMAND ${GCOVR_PATH} 
                -r ${CMAKE_SOURCE_DIR} 
                --filter "${CMAKE_SOURCE_DIR}/src/"
                --filter "${CMAKE_SOURCE_DIR}/include/"
                --exclude "${CMAKE_BINARY_DIR}"
                --exclude "${CMAKE_SOURCE_DIR}/tests/"
                --exclude "${CMAKE_SOURCE_DIR}/benchmarks/"
                --exclude "${CMAKE_SOURCE_DIR}/src/main.cpp"
                --gcov-ignore-parse-errors
                --gcov-ignore-errors=all
                --exclude-throw-branches
                --html --html-details 
                -o ${CMAKE_BINARY_DIR}/coverage/index.html

            # 4. Generate XML Report (For Tools/Codecov)
            COMMAND ${GCOVR_PATH} 
                -r ${CMAKE_SOURCE_DIR} 
                --filter "${CMAKE_SOURCE_DIR}/src/"
                --filter "${CMAKE_SOURCE_DIR}/include/"
                --exclude "${CMAKE_BINARY_DIR}"
                --exclude "${CMAKE_SOURCE_DIR}/tests/"
                --exclude "${CMAKE_SOURCE_DIR}/benchmarks/"
                --exclude "${CMAKE_SOURCE_DIR}/src/main.cpp"
                --gcov-ignore-parse-errors
                --gcov-ignore-errors=all
                --exclude-throw-branches
                --xml 
                -o ${CMAKE_BINARY_DIR}/coverage.xml
            
                # 5. Print Summary to Console (Standard Output)
            COMMAND ${GCOVR_PATH} 
                -r ${CMAKE_SOURCE_DIR} 
                --filter "${CMAKE_SOURCE_DIR}/src/"
                --filter "${CMAKE_SOURCE_DIR}/include/"
                --exclude "${CMAKE_BINARY_DIR}"
                --exclude "${CMAKE_SOURCE_DIR}/tests/"
                --exclude "${CMAKE_SOURCE_DIR}/benchmarks/"
                --exclude "${CMAKE_SOURCE_DIR}/src/main.cpp"
                --gcov-ignore-parse-errors
                --gcov-ignore-errors=all
                --exclude-throw-branches
                --txt # explicitly sets text format
            
            COMMAND ${CMAKE_COMMAND} -E echo "--------------------------------------------------"
            COMMAND ${CMAKE_COMMAND} -E echo "Coverage Filter: ${CMAKE_SOURCE_DIR}/src/"
            COMMAND ${CMAKE_COMMAND} -E echo "Coverage Filter: ${CMAKE_SOURCE_DIR}/src/"
            COMMAND ${CMAKE_COMMAND} -E echo "Exclude Path 1:  ${CMAKE_BINARY_DIR}"
            COMMAND ${CMAKE_COMMAND} -E echo "Exclude Path 2:  ${CMAKE_SOURCE_DIR}/tests/"
            COMMAND ${CMAKE_COMMAND} -E echo "Exclude Path 3:  ${CMAKE_SOURCE_DIR}/benchmarks/"
            COMMAND ${CMAKE_COMMAND} -E echo "Exclude Path 3:  ${CMAKE_SOURCE_DIR}/src/main.cpp"
            COMMAND ${CMAKE_COMMAND} -E echo "--------------------------------------------------"
            

            
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Generating coverage report..."
        )
    endif()
endif()



add_executable(unit_tests 
    test_discrete.cpp
    test_dense.cpp
    test_interval_set.cpp
    test_readers.cpp
    test_parse.cpp
    test_json_input.cpp
)

target_link_libraries(unit_tests PRIVATE loomrv_lib Catch2::Catch2WithMain simdjson)
add_dependencies(unit_tests download_testdata)

# Copy Data: Root/data -> build/tests/data
add_custom_command(TARGET unit_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data
    $<TARGET_FILE_DIR:unit_tests>/data
)

include(CTest)
include(Catch)
