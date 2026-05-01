#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_all.hpp>

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "loomrv/json_reader.hpp"
#include "loomrv/binary_row_reader.hpp"


#include "loomrv/MTLEngine.hpp"
#include "loomrv/ptl.hpp"


TEST_CASE("Discrete Implementation Tests", "[discrete]") {

    using namespace loomrv;
    using namespace db_interval_set;

    SECTION("2 Eventually") {
        std::vector<std::vector<bool>> propositionInputs = {
        {true, false, false, false, false, false},    // p
        {false, false, false, false, true, false}  // q
        };
        std::vector<bool> expectedOutput = {false, false, true, true, true, false};

        ptl_parser parser;
        auto monitor = createDiscreteMultiPropertyMonitor(1000);
        parser.parse_discrete("once[1:2](once[1:2]({p} || {q}))", monitor);
        finalize_monitor(monitor, {"p", "q"});

        bool allCorrect = true;
        for(int time = 0; time < propositionInputs[0].size(); time++) {
            auto outputs = eval_multi_property(monitor, time, {propositionInputs[0][time], propositionInputs[1][time]});
            allCorrect &= outputs[0] == expectedOutput[time];
            
        }
        REQUIRE(allCorrect == true);
        
    }


    SECTION("Always") {
        std::vector<std::vector<bool>> propositionInputs = {
        {false, false, true, true, true, false},    // p
        };
        std::vector<bool> expectedOutput = {true, false, false, false, true, true};

        ptl_parser parser;
        auto monitor = createDiscreteMultiPropertyMonitor(1000);
        parser.parse_discrete("H[1:2]{p}", monitor);
        finalize_monitor(monitor, {"p"});

        bool allCorrect = true;
        for(int time = 0; time < propositionInputs[0].size(); time++) {
            auto outputs = eval_multi_property(monitor, time, std::vector<bool>{propositionInputs[0][time]});
            allCorrect &= outputs[0] == expectedOutput[time];
            
        }
        REQUIRE(allCorrect == true);
    }

    SECTION("Since") {
        std::vector<std::vector<bool>> propositionInputs = {
        {false, false, true, true, true, false},    // p
        {false, true, false, false, true, false},   // q
        };
        std::vector<bool> expectedOutput = {false, false, false, true, true, false};

        ptl_parser parser;
        auto monitor = createDiscreteMultiPropertyMonitor(1000);
        parser.parse_discrete("{p} S[2:3] {q}", monitor);
        finalize_monitor(monitor, {"p", "q"});

        bool allCorrect = true;
        for(int time = 0; time < propositionInputs[0].size(); time++) {
            auto outputs = eval_multi_property(monitor, time, {propositionInputs[0][time], propositionInputs[1][time]});
            allCorrect &= outputs[0] == expectedOutput[time];
            
        }
        REQUIRE(allCorrect == true);
    }
}




TEST_CASE("Discrete AbsentAQ", "[discrete][AbsentAQ]") {

    using namespace db_interval_set;
    using namespace loomrv;


     // --- 1. SETUP PARAMS ---
    auto params = GENERATE(table<std::string, int>({
        {"Discrete", 10}, 
        {"Discrete", 100}, 
        {"Discrete", 1000}, 
    }));

    // Unpack parameters
    std::string CONDENSATION;
    int TIMINGS;
    std::tie(CONDENSATION, TIMINGS) = params;

    std::string benchmarkName = CONDENSATION + " " + std::to_string(TIMINGS);

    SECTION("AbsentAQ " + benchmarkName) {
        ptl_parser parser;
        auto monitor = createDiscreteMultiPropertyMonitor(1000);
        parser.parse_discrete("historically((once[:" + std::to_string(TIMINGS) + "]{q}) -> ((not{p}) since {q}))", monitor);
        finalize_monitor(monitor, {"q", "p"});

        std::string file_name = "data/fullsuite/AbsentAQ/" + CONDENSATION + "/1M/AbsentAQ" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = binary_row_reader::readInputFile(file_name);
        bool all_correct = true;
        int maxHolderUsage = 0;
        for (int i = 0; i < allInputs.size(); i++){
            auto outputs = eval_multi_property(monitor, allInputs[i].time, {allInputs[i].q, allInputs[i].p});
            if (!outputs[0]) {
                all_correct = false;
                std::cout << benchmarkName << std::endl;
                break;
            }
            maxHolderUsage = std::max(maxHolderUsage, monitor.holder.writeIndex);
            
        }
        std::cout << "Usage: " << maxHolderUsage << std::endl;
        REQUIRE(all_correct == true);

    };
}



TEST_CASE("Discrete AbsentBQR", "[discrete][AbsentBQR]") {
    using namespace db_interval_set;
    using namespace loomrv;

    auto params = GENERATE(table<std::string, int>({
        {"Discrete", 10}, 
        {"Discrete", 100}, 
        {"Discrete", 1000}, 
    }));

    std::string CONDENSATION;
    int TIMINGS;
    std::tie(CONDENSATION, TIMINGS) = params;

    std::string benchmarkName = CONDENSATION + " " + std::to_string(TIMINGS);

    SECTION("AbsentBQR " + benchmarkName) {
        const int since_a = 3 * (TIMINGS / 10);
        const int since_b = TIMINGS;

        ptl_parser parser;
        auto monitor = createDiscreteMultiPropertyMonitor(1000);
        parser.parse_discrete("historically(({r} && !{q} && once{q}) -> ((not{p}) since[" + std::to_string(since_a) + ":" + std::to_string(since_b) + "] {q}))", monitor);
        finalize_monitor(monitor, {"q", "p", "r"});

        std::string file_name = "data/fullsuite/AbsentBQR/" + CONDENSATION + "/1M/AbsentBQR" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = binary_row_reader::readInputFile(file_name);
        
        bool all_correct = true;
        int maxHolderUsage = 0;

        for (int i = 0; i < allInputs.size(); i++){
            auto outputs = eval_multi_property(monitor, allInputs[i].time, {allInputs[i].q, allInputs[i].p, allInputs[i].r});
            if (!outputs[0]) {
                all_correct = false;
                std::cout << benchmarkName << std::endl;
                break;
            }
            maxHolderUsage = std::max(maxHolderUsage, monitor.holder.writeIndex);
            
        }
        std::cout << "AbsentBQR Usage: " << maxHolderUsage << std::endl;
        REQUIRE(all_correct == true);
    };
}

TEST_CASE("Discrete AbsentBR", "[discrete][AbsentBR]") {
    using namespace db_interval_set;
    using namespace loomrv;

    auto params = GENERATE(table<std::string, int>({
        {"Discrete", 10}, 
        {"Discrete", 100}, 
        {"Discrete", 1000}, 
    }));

    std::string CONDENSATION;
    int TIMINGS;
    std::tie(CONDENSATION, TIMINGS) = params;

    std::string benchmarkName = CONDENSATION + " " + std::to_string(TIMINGS);

    SECTION("AbsentBR " + benchmarkName) {
        ptl_parser parser;
        auto monitor = createDiscreteMultiPropertyMonitor(1000);
        parser.parse_discrete("historically({r} -> (historically[:" + std::to_string(TIMINGS) + "](not{p})))", monitor);
        finalize_monitor(monitor, {"q", "p", "r"});

        std::string file_name = "data/fullsuite/AbsentBR/" + CONDENSATION + "/1M/AbsentBR" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = binary_row_reader::readInputFile(file_name);
        
        bool all_correct = true;
        int maxHolderUsage = 0;

        for (int i = 0; i < allInputs.size(); i++){
            auto outputs = eval_multi_property(monitor, allInputs[i].time, {allInputs[i].q, allInputs[i].p, allInputs[i].r});
            if (!outputs[0]) {
                all_correct = false;
                std::cout << benchmarkName << std::endl;
                break;
            }
            maxHolderUsage = std::max(maxHolderUsage, monitor.holder.writeIndex);
            
        }
        std::cout << "AbsentBR Usage: " << maxHolderUsage << std::endl;
        REQUIRE(all_correct == true);
    };
}

TEST_CASE("Discrete AlwaysAQ", "[discrete][AlwaysAQ]") {
    using namespace db_interval_set;
    using namespace loomrv;

    auto params = GENERATE(table<std::string, int>({
        {"Discrete", 10}, 
        {"Discrete", 100}, 
        {"Discrete", 1000}, 
    }));

    std::string CONDENSATION;
    int TIMINGS;
    std::tie(CONDENSATION, TIMINGS) = params;

    std::string benchmarkName = CONDENSATION + " " + std::to_string(TIMINGS);

    SECTION("AlwaysAQ " + benchmarkName) {
        ptl_parser parser;
        auto monitor = createDiscreteMultiPropertyMonitor(1000);
        parser.parse_discrete("historically((once[:" + std::to_string(TIMINGS) + "]{q}) -> ({p} since {q}))", monitor);
        finalize_monitor(monitor, {"q", "p", "r"});

        std::string file_name = "data/fullsuite/AlwaysAQ/" + CONDENSATION + "/1M/AlwaysAQ" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = binary_row_reader::readInputFile(file_name);
        
        bool all_correct = true;
        int maxHolderUsage = 0;

        for (int i = 0; i < allInputs.size(); i++){
            auto outputs = eval_multi_property(monitor, allInputs[i].time, {allInputs[i].q, allInputs[i].p, allInputs[i].r});
            if (!outputs[0]) {
                all_correct = false;
                std::cout << benchmarkName << std::endl;
                break;
            }
            maxHolderUsage = std::max(maxHolderUsage, monitor.holder.writeIndex);
            
        }
        std::cout << "AlwaysAQ Usage: " << maxHolderUsage << std::endl;
        REQUIRE(all_correct == true);
    };
}

TEST_CASE("Discrete AlwaysBQR", "[discrete][AlwaysBQR]") {
    using namespace db_interval_set;
    using namespace loomrv;

    auto params = GENERATE(table<std::string, int>({
        {"Discrete", 10}, 
        {"Discrete", 100}, 
        {"Discrete", 1000}, 
    }));

    std::string CONDENSATION;
    int TIMINGS;
    std::tie(CONDENSATION, TIMINGS) = params;

    std::string benchmarkName = CONDENSATION + " " + std::to_string(TIMINGS);

    SECTION("AlwaysBQR " + benchmarkName) {
        const int since_a = 3 * (TIMINGS / 10);
        const int since_b = TIMINGS;

        ptl_parser parser;
        auto monitor = createDiscreteMultiPropertyMonitor(1000);
        parser.parse_discrete("historically(({r} && !{q} && once{q}) -> ({p} since[" + std::to_string(since_a) + ":" + std::to_string(since_b) + "] {q}))", monitor);
        finalize_monitor(monitor, {"q", "p", "r"});

        std::string file_name = "data/fullsuite/AlwaysBQR/" + CONDENSATION + "/1M/AlwaysBQR" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = binary_row_reader::readInputFile(file_name);
        
        bool all_correct = true;
        int maxHolderUsage = 0;

        for (int i = 0; i < allInputs.size(); i++){
            auto outputs = eval_multi_property(monitor, allInputs[i].time, {allInputs[i].q, allInputs[i].p, allInputs[i].r});
            if (!outputs[0]) {
                all_correct = false;
                std::cout << benchmarkName << std::endl;
                break;
            }
            maxHolderUsage = std::max(maxHolderUsage, monitor.holder.writeIndex);
            
        }
        std::cout << "AlwaysBQR Usage: " << maxHolderUsage << std::endl;
        REQUIRE(all_correct == true);
    };
}

TEST_CASE("Discrete AlwaysBR", "[discrete][AlwaysBR]") {
    using namespace db_interval_set;
    using namespace loomrv;

    auto params = GENERATE(table<std::string, int>({
        {"Discrete", 10}, 
        {"Discrete", 100}, 
        {"Discrete", 1000}, 
    }));

    std::string CONDENSATION;
    int TIMINGS;
    std::tie(CONDENSATION, TIMINGS) = params;

    std::string benchmarkName = CONDENSATION + " " + std::to_string(TIMINGS);

    SECTION("AlwaysBR " + benchmarkName) {
        ptl_parser parser;
        auto monitor = createDiscreteMultiPropertyMonitor(1000);
        parser.parse_discrete("historically({r} -> (historically[:" + std::to_string(TIMINGS) + "]{p}))", monitor);
        finalize_monitor(monitor, {"q", "p", "r"});

        std::string file_name = "data/fullsuite/AlwaysBR/" + CONDENSATION + "/1M/AlwaysBR" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = binary_row_reader::readInputFile(file_name);
        
        bool all_correct = true;
        int maxHolderUsage = 0;

        for (int i = 0; i < allInputs.size(); i++){
            auto outputs = eval_multi_property(monitor, allInputs[i].time, {allInputs[i].q, allInputs[i].p, allInputs[i].r});
            if (!outputs[0]) {
                all_correct = false;
                std::cout << benchmarkName << std::endl;
                break;
            }
            maxHolderUsage = std::max(maxHolderUsage, monitor.holder.writeIndex);
            
        }
        std::cout << "AlwaysBR Usage: " << maxHolderUsage << std::endl;
        REQUIRE(all_correct == true);
    };
}

TEST_CASE("Discrete RecurBQR", "[discrete][RecurBQR]") {
    using namespace db_interval_set;
    using namespace loomrv;

    auto params = GENERATE(table<std::string, int>({
        {"Discrete", 10}, 
        {"Discrete", 100}, 
        {"Discrete", 1000}, 
    }));

    std::string CONDENSATION;
    int TIMINGS;
    std::tie(CONDENSATION, TIMINGS) = params;

    std::string benchmarkName = CONDENSATION + " " + std::to_string(TIMINGS);

    SECTION("RecurBQR " + benchmarkName) {
        ptl_parser parser;
        auto monitor = createDiscreteMultiPropertyMonitor(1000);
        parser.parse_discrete("historically(({r} && !{q} && once{q}) -> ((once[:" + std::to_string(TIMINGS) + "]({p} or {q})) since {q}))", monitor);
        finalize_monitor(monitor, {"q", "p", "r"});

        std::string file_name = "data/fullsuite/RecurBQR/" + CONDENSATION + "/1M/RecurBQR" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = binary_row_reader::readInputFile(file_name);
        
        bool all_correct = true;
        int maxHolderUsage = 0;

        for (int i = 0; i < allInputs.size(); i++){
            auto outputs = eval_multi_property(monitor, allInputs[i].time, {allInputs[i].q, allInputs[i].p, allInputs[i].r});
            if (!outputs[0]) {
                all_correct = false;
                std::cout << benchmarkName << std::endl;
                break;
            }
            maxHolderUsage = std::max(maxHolderUsage, monitor.holder.writeIndex);
            
        }
        std::cout << "RecurBQR Usage: " << maxHolderUsage << std::endl;
        REQUIRE(all_correct == true);
    };
}

TEST_CASE("Discrete RecurGLB", "[discrete][RecurGLB]") {
    using namespace db_interval_set;
    using namespace loomrv;

    auto params = GENERATE(table<std::string, int>({
        {"Discrete", 10}, 
        {"Discrete", 100}, 
        {"Discrete", 1000}, 
    }));

    std::string CONDENSATION;
    int TIMINGS;
    std::tie(CONDENSATION, TIMINGS) = params;

    std::string benchmarkName = CONDENSATION + " " + std::to_string(TIMINGS);

    SECTION("RecurGLB " + benchmarkName) {
        ptl_parser parser;
        auto monitor = createDiscreteMultiPropertyMonitor(1000);
        parser.parse_discrete("historically(once[:" + std::to_string(TIMINGS) + "]{p})", monitor);
        finalize_monitor(monitor, {"p"});

        std::string file_name = "data/fullsuite/RecurGLB/" + CONDENSATION + "/1M/RecurGLB" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = binary_row_reader::readInputFile(file_name);
        
        bool all_correct = true;
        int maxHolderUsage = 0;

        for (int i = 0; i < allInputs.size(); i++){
            auto outputs = eval_multi_property(monitor, allInputs[i].time, std::vector<bool>{allInputs[i].p});
            if (!outputs[0]) {
                all_correct = false;
                std::cout << benchmarkName << std::endl;
                break;
            }
            maxHolderUsage = std::max(maxHolderUsage, monitor.holder.writeIndex);
            
        }
        std::cout << "RecurGLB Usage: " << maxHolderUsage << std::endl;
        REQUIRE(all_correct == true);
    };
}

TEST_CASE("Discrete RespondBQR", "[discrete][RespondBQR]") {
    using namespace db_interval_set;
    using namespace loomrv;

    auto params = GENERATE(table<std::string, int>({
        {"Discrete", 10}, 
        {"Discrete", 100}, 
        {"Discrete", 1000}, 
    }));

    std::string CONDENSATION;
    int TIMINGS;
    std::tie(CONDENSATION, TIMINGS) = params;

    std::string benchmarkName = CONDENSATION + " " + std::to_string(TIMINGS);

    SECTION("RespondBQR " + benchmarkName) {
        const int once_a = 3 * (TIMINGS / 10);
        const int once_b = TIMINGS;
        const int since_a = TIMINGS;

        ptl_parser parser;
        auto monitor = createDiscreteMultiPropertyMonitor(1000);
        parser.parse_discrete("historically(({r} && !{q} && once{q}) -> ( (({s} -> once[" + std::to_string(once_a) + ":" + std::to_string(once_b) + "]{p}) and not((not {s}) since[" + std::to_string(since_a) + ":] {p})) since {q}))", monitor);
        finalize_monitor(monitor, {"q", "p", "s", "r"});

        std::string file_name = "data/fullsuite/RespondBQR/" + CONDENSATION + "/1M/RespondBQR" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = binary_row_reader::readInputFile(file_name);
        
        bool all_correct = true;
        int maxHolderUsage = 0;

        for (int i = 0; i < allInputs.size(); i++){
            auto outputs = eval_multi_property(monitor, allInputs[i].time, {allInputs[i].q, allInputs[i].p, allInputs[i].s, allInputs[i].r});
            if (!outputs[0]) {
                all_correct = false;
                std::cout << benchmarkName << std::endl;
                break;
            }
            maxHolderUsage = std::max(maxHolderUsage, monitor.holder.writeIndex);
            
        }
        std::cout << "RespondBQR Usage: " << maxHolderUsage << std::endl;
        REQUIRE(all_correct == true);
    };
}

TEST_CASE("Discrete RespondGLB", "[discrete][RespondGLB]") {
    using namespace db_interval_set;
    using namespace loomrv;

    auto params = GENERATE(table<std::string, int>({
        {"Discrete", 10}, 
        {"Discrete", 100}, 
        {"Discrete", 1000}, 
    }));

    std::string CONDENSATION;
    int TIMINGS;
    std::tie(CONDENSATION, TIMINGS) = params;

    std::string benchmarkName = CONDENSATION + " " + std::to_string(TIMINGS);

    SECTION("RespondGLB " + benchmarkName) {
        const int once_a = 3 * (TIMINGS / 10);
        const int once_b = TIMINGS;
        const int since_a = TIMINGS;

        ptl_parser parser;
        auto monitor = createDiscreteMultiPropertyMonitor(1000);
        parser.parse_discrete("historically(({s} -> once[" + std::to_string(once_a) + ":" + std::to_string(once_b) + "]{p}) and not((not {s}) since[" + std::to_string(since_a) + ":] {p}))", monitor);
        finalize_monitor(monitor, {"p", "s"});

        std::string file_name = "data/fullsuite/RespondGLB/" + CONDENSATION + "/1M/RespondGLB" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = binary_row_reader::readInputFile(file_name);
        
        bool all_correct = true;
        int maxHolderUsage = 0;

        for (int i = 0; i < allInputs.size(); i++){
            auto outputs = eval_multi_property(monitor, allInputs[i].time, {allInputs[i].p, allInputs[i].s});
            if (!outputs[0]) {
                all_correct = false;
                std::cout << benchmarkName << std::endl;
                break;
            }
            maxHolderUsage = std::max(maxHolderUsage, monitor.holder.writeIndex);
            
        }
        std::cout << "RespondGLB Usage: " << maxHolderUsage << std::endl;
        REQUIRE(all_correct == true);
    };
}