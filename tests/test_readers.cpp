#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_all.hpp>

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "loomrv/json_reader.hpp"
#include "loomrv/binary_row_reader.hpp"


TEST_CASE("Readers Implementation Tests", "[reader]") {
    const auto &allInputs = binary_row_reader::readInputFile("data/fullsuite/AbsentAQ/Dense10/1M/AbsentAQ10.row.bin");

    int i = 0;
    std::ifstream input_file("data/fullsuite/AbsentAQ/Dense10/1M/AbsentAQ10.jsonl");
    for (std::string line; std::getline(input_file, line);){
        auto newInput = json_reader::read_line(line);
        bool q = newInput.propositions[0];
        bool p = newInput.propositions[1];
        int time = newInput.time;
        REQUIRE(((allInputs[i].q == q) && (allInputs[i].p == p) && (allInputs[i].time == time)));
        i++;
    }

}