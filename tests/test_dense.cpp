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
TEST_CASE("Dense Implementation tests", "[dense]") {
    using namespace std;
    using namespace loomrv;
    using namespace db_interval_set;
    
    auto holder = newHolder(1000);
    vector<DenseNode> nodes;
    auto p = DenseNode{empty(holder), createSetFromIntervals(holder, {{7, 30}}), NodeType::TEST, 0, 0, 0, 0};
    nodes.push_back(p);
    auto q = DenseNode{empty(holder), createSetFromIntervals(holder, {{3, 8}}), NodeType::TEST, 0, 0, 0, 0};
    nodes.push_back(q);
    auto since = DenseNode{empty(holder), empty(holder), NodeType::SINCE, 0, 1, 18, 24};
    nodes.push_back(since);

    auto out = run_evaluation(nodes, holder, 0, 30, {true, true});
    auto converted = toVectorIntervals(out);
    REQUIRE(converted == std::vector<Interval>{{25, 30}});
    swapBuffers(holder);

    nodes[0].output = createSetFromIntervals(holder, {{30,35},{39,47}});
    nodes[1].output = createSetFromIntervals(holder, {{38,39}});
    out = run_evaluation(nodes, holder, 30, 47, {true, true});
    converted = toVectorIntervals(out);
    REQUIRE(converted == std::vector<Interval>{{30, 32}});
    swapBuffers(holder);

    nodes[0].output = createSetFromIntervals(holder, {{47,49},{63,75}});
    nodes[1].output = createSetFromIntervals(holder, {{70,75}});
    out = run_evaluation(nodes, holder, 47, 75, {true, true});
    converted = toVectorIntervals(out);
    REQUIRE(converted == std::vector<Interval>{});

    swapBuffers(holder);

    nodes[0].output = createSetFromIntervals(holder, {{75,99}});
    nodes[1].output = createSetFromIntervals(holder, {{75,89}});
    out = run_evaluation(nodes, holder, 75, 99, {true, true});
    converted = toVectorIntervals(out);
    REQUIRE(converted == std::vector<Interval>{{88, 99}});

    swapBuffers(holder);
    destroyHolder(holder);
}



TEST_CASE("Dense Timescales Tests", "[dense][old]") {

    using namespace db_interval_set;
    using namespace loomrv;

    ptl_parser parser;
    auto monitor = createDenseMultiPropertyMonitor(1000);
    parser.parse_dense("historically(once[:10]{p})", monitor);
    finalize_monitor(monitor, {"p"});

    std::ifstream input_file("data/fullsuite/RecurGLB/Dense10/1M/RecurGLB10.jsonl");
    json_reader::TimescalesInput prevInput;
    int step = 0;
    bool all_correct = true;

    for (std::string line; std::getline(input_file, line);){
        if (step != 0) {
            auto newInput = json_reader::read_line(line);
            auto outputs = eval_multi_property(monitor, prevInput.time, newInput.time, prevInput.propositions);
            if (toVectorIntervals(outputs[0]) != std::vector<Interval>{{prevInput.time, newInput.time}}) {
                all_correct = false;
            }
            prevInput = newInput;
        }
        else {
            prevInput = json_reader::read_line(line);
        }
        step++;
    };

    REQUIRE(all_correct);

}



TEST_CASE("Dense AbsentAQ", "[dense][AbsentAQ]") {

    using namespace db_interval_set;
    using namespace loomrv;


     // --- 1. SETUP PARAMS ---
    auto params = GENERATE(table<std::string, int>({
        {"Dense10", 10},
        {"Dense100", 10}, 
        {"Discrete", 10}, 
        {"Dense10", 100},
        {"Dense100", 100}, 
        {"Discrete", 100}, 
        {"Dense10", 1000},
        {"Dense100", 1000}, 
        {"Discrete", 1000}, 
    }));

    // Unpack parameters
    std::string CONDENSATION;
    int TIMINGS;
    std::tie(CONDENSATION, TIMINGS) = params;

    std::string benchmarkName = CONDENSATION + " " + std::to_string(TIMINGS);

    SECTION("AbsentAQ " + benchmarkName) {
        ptl_parser parser;
        auto monitor = createDenseMultiPropertyMonitor(1000);
        parser.parse_dense("historically((once[:" + std::to_string(TIMINGS) + "]{q}) -> ((not{p}) since {q}))", monitor);
        finalize_monitor(monitor, {"q", "p"});

        std::string file_name = "data/fullsuite/AbsentAQ/" + CONDENSATION + "/1M/AbsentAQ" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = binary_row_reader::readInputFile(file_name);
        bool all_correct = true;
        int maxHolderUsage = 0;
        for (int i = 1; i < allInputs.size(); i++){
            auto outputs = eval_multi_property(monitor, allInputs[i - 1].time, allInputs[i].time, {allInputs[i - 1].q, allInputs[i - 1].p});
            if (toVectorIntervals(outputs[0]) != std::vector<Interval>{{allInputs[i - 1].time, allInputs[i].time}}) {
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


TEST_CASE("Dense AbsentBQR", "[dense][AbsentBQR]") {
    using namespace db_interval_set;
    using namespace loomrv;

    auto params = GENERATE(table<std::string, int>({
        {"Dense10", 10}, {"Dense100", 10}, {"Discrete", 10}, 
        {"Dense10", 100}, {"Dense100", 100}, {"Discrete", 100}, 
        {"Dense10", 1000}, {"Dense100", 1000}, {"Discrete", 1000}, 
    }));

    std::string CONDENSATION;
    int TIMINGS;
    std::tie(CONDENSATION, TIMINGS) = params;

    std::string benchmarkName = CONDENSATION + " " + std::to_string(TIMINGS);

    SECTION("AbsentBQR " + benchmarkName) {
        const int since_a = 3 * (TIMINGS / 10);
        const int since_b = TIMINGS;

        ptl_parser parser;
        auto monitor = createDenseMultiPropertyMonitor(1000);    
        parser.parse_dense("historically(({r} && !{q} && once{q}) -> ((not{p}) since[" + std::to_string(since_a) + ":" + std::to_string(since_b) + "] {q}))", monitor);
        finalize_monitor(monitor, {"q", "p", "r"}); 

        std::string file_name = "data/fullsuite/AbsentBQR/" + CONDENSATION + "/1M/AbsentBQR" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = binary_row_reader::readInputFile(file_name);
        
        bool all_correct = true;
        int maxHolderUsage = 0;

        for (int i = 1; i < allInputs.size(); i++){
            auto outputs = eval_multi_property(monitor, allInputs[i - 1].time, allInputs[i].time, {allInputs[i - 1].q, allInputs[i - 1].p, allInputs[i - 1].r});
            
            if (toVectorIntervals(outputs[0]) != std::vector<Interval>{{allInputs[i - 1].time, allInputs[i].time}}) {
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

TEST_CASE("Dense AbsentBR", "[dense][AbsentBR]") {
    using namespace db_interval_set;
    using namespace loomrv;

    auto params = GENERATE(table<std::string, int>({
        {"Dense10", 10}, {"Dense100", 10}, {"Discrete", 10}, 
        {"Dense10", 100}, {"Dense100", 100}, {"Discrete", 100}, 
        {"Dense10", 1000}, {"Dense100", 1000}, {"Discrete", 1000}, 
    }));

    std::string CONDENSATION;
    int TIMINGS;
    std::tie(CONDENSATION, TIMINGS) = params;

    std::string benchmarkName = CONDENSATION + " " + std::to_string(TIMINGS);

    SECTION("AbsentBR " + benchmarkName) {
        ptl_parser parser;
        auto monitor = createDenseMultiPropertyMonitor(1000);
        parser.parse_dense("historically({r} -> (historically[:" + std::to_string(TIMINGS) + "](not{p})))", monitor);
        finalize_monitor(monitor, {"q", "p", "r"});

        std::string file_name = "data/fullsuite/AbsentBR/" + CONDENSATION + "/1M/AbsentBR" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = binary_row_reader::readInputFile(file_name);
        
        bool all_correct = true;
        int maxHolderUsage = 0;

        for (int i = 1; i < allInputs.size(); i++){
            auto outputs = eval_multi_property(monitor, allInputs[i - 1].time, allInputs[i].time, {allInputs[i - 1].q, allInputs[i - 1].p, allInputs[i - 1].r});
            
            if (toVectorIntervals(outputs[0]) != std::vector<Interval>{{allInputs[i - 1].time, allInputs[i].time}}) {
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


TEST_CASE("Dense AlwaysAQ", "[dense][AlwaysAQ]") {
    using namespace db_interval_set;
    using namespace loomrv;

    auto params = GENERATE(table<std::string, int>({
        {"Dense10", 10}, {"Dense100", 10}, {"Discrete", 10}, 
        {"Dense10", 100}, {"Dense100", 100}, {"Discrete", 100}, 
        {"Dense10", 1000}, {"Dense100", 1000}, {"Discrete", 1000}, 
    }));

    std::string CONDENSATION;
    int TIMINGS;
    std::tie(CONDENSATION, TIMINGS) = params;

    std::string benchmarkName = CONDENSATION + " " + std::to_string(TIMINGS);

    SECTION("AlwaysAQ " + benchmarkName) {
        ptl_parser parser;
        auto monitor = createDenseMultiPropertyMonitor(1000);
        parser.parse_dense("historically((once[:" + std::to_string(TIMINGS) + "]{q}) -> ({p} since {q}))", monitor);
        finalize_monitor(monitor, {"q", "p", "r"});

        std::string file_name = "data/fullsuite/AlwaysAQ/" + CONDENSATION + "/1M/AlwaysAQ" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = binary_row_reader::readInputFile(file_name);
        
        bool all_correct = true;
        int maxHolderUsage = 0;

        for (int i = 1; i < allInputs.size(); i++){
            auto outputs = eval_multi_property(monitor, allInputs[i - 1].time, allInputs[i].time, {allInputs[i - 1].q, allInputs[i - 1].p, allInputs[i - 1].r});
            
            if (toVectorIntervals(outputs[0]) != std::vector<Interval>{{allInputs[i - 1].time, allInputs[i].time}}) {
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

TEST_CASE("Dense AlwaysBQR", "[dense][AlwaysBQR]") {
    using namespace db_interval_set;
    using namespace loomrv;

    auto params = GENERATE(table<std::string, int>({
        {"Dense10", 10}, {"Dense100", 10}, {"Discrete", 10}, 
        {"Dense10", 100}, {"Dense100", 100}, {"Discrete", 100}, 
        {"Dense10", 1000}, {"Dense100", 1000}, {"Discrete", 1000}, 
    }));

    std::string CONDENSATION;
    int TIMINGS;
    std::tie(CONDENSATION, TIMINGS) = params;

    std::string benchmarkName = CONDENSATION + " " + std::to_string(TIMINGS);

    SECTION("AlwaysBQR " + benchmarkName) {
        const int since_a = 3 * (TIMINGS / 10);
        const int since_b = TIMINGS;

        ptl_parser parser;
        auto monitor = createDenseMultiPropertyMonitor(1000);
        parser.parse_dense("historically(({r} && !{q} && once{q}) -> ({p} since[" + std::to_string(since_a) + ":" + std::to_string(since_b) + "] {q}))", monitor);
        finalize_monitor(monitor, {"q", "p", "r"});

        std::string file_name = "data/fullsuite/AlwaysBQR/" + CONDENSATION + "/1M/AlwaysBQR" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = binary_row_reader::readInputFile(file_name);
        
        bool all_correct = true;
        int maxHolderUsage = 0;

        for (int i = 1; i < allInputs.size(); i++){
            auto outputs = eval_multi_property(monitor, allInputs[i - 1].time, allInputs[i].time, {allInputs[i - 1].q, allInputs[i - 1].p, allInputs[i - 1].r});
            
            if (toVectorIntervals(outputs[0]) != std::vector<Interval>{{allInputs[i - 1].time, allInputs[i].time}}) {
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

TEST_CASE("Dense AlwaysBR", "[dense][AlwaysBR]") {
    using namespace db_interval_set;
    using namespace loomrv;

    auto params = GENERATE(table<std::string, int>({
        {"Dense10", 10}, {"Dense100", 10}, {"Discrete", 10}, 
        {"Dense10", 100}, {"Dense100", 100}, {"Discrete", 100}, 
        {"Dense10", 1000}, {"Dense100", 1000}, {"Discrete", 1000}, 
    }));

    std::string CONDENSATION;
    int TIMINGS;
    std::tie(CONDENSATION, TIMINGS) = params;

    std::string benchmarkName = CONDENSATION + " " + std::to_string(TIMINGS);

    SECTION("AlwaysBR " + benchmarkName) {
        ptl_parser parser;
        auto monitor = createDenseMultiPropertyMonitor(1000);
        parser.parse_dense("historically({r} -> (historically[:" + std::to_string(TIMINGS) + "]{p}))", monitor);
        finalize_monitor(monitor, {"q", "p", "r"});

        std::string file_name = "data/fullsuite/AlwaysBR/" + CONDENSATION + "/1M/AlwaysBR" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = binary_row_reader::readInputFile(file_name);
        
        bool all_correct = true;
        int maxHolderUsage = 0;

        for (int i = 1; i < allInputs.size(); i++){
            auto outputs = eval_multi_property(monitor, allInputs[i - 1].time, allInputs[i].time, {allInputs[i - 1].q, allInputs[i - 1].p, allInputs[i - 1].r});
            
            if (toVectorIntervals(outputs[0]) != std::vector<Interval>{{allInputs[i - 1].time, allInputs[i].time}}) {
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

TEST_CASE("Dense RecurBQR", "[dense][RecurBQR]") {
    using namespace db_interval_set;
    using namespace loomrv;

    auto params = GENERATE(table<std::string, int>({
        {"Dense10", 10}, {"Dense100", 10}, {"Discrete", 10}, 
        {"Dense10", 100}, {"Dense100", 100}, {"Discrete", 100}, 
        {"Dense10", 1000}, {"Dense100", 1000}, {"Discrete", 1000}, 
    }));

    std::string CONDENSATION;
    int TIMINGS;
    std::tie(CONDENSATION, TIMINGS) = params;

    std::string benchmarkName = CONDENSATION + " " + std::to_string(TIMINGS);

    SECTION("RecurBQR " + benchmarkName) {
        ptl_parser parser;
        auto monitor = createDenseMultiPropertyMonitor(1000);
        parser.parse_dense("historically(({r} && !{q} && once{q}) -> ((once[:" + std::to_string(TIMINGS) + "]({p} or {q})) since {q}))", monitor);
        finalize_monitor(monitor, {"q", "p", "r"});

        std::string file_name = "data/fullsuite/RecurBQR/" + CONDENSATION + "/1M/RecurBQR" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = binary_row_reader::readInputFile(file_name);
        
        bool all_correct = true;
        int maxHolderUsage = 0;

        for (int i = 1; i < allInputs.size(); i++){
            auto outputs = eval_multi_property(monitor, allInputs[i - 1].time, allInputs[i].time, {allInputs[i - 1].q, allInputs[i - 1].p, allInputs[i - 1].r});
            
            if (toVectorIntervals(outputs[0]) != std::vector<Interval>{{allInputs[i - 1].time, allInputs[i].time}}) {
                all_correct = false;
            }
            maxHolderUsage = std::max(maxHolderUsage, monitor.holder.writeIndex);
            
        }
        std::cout << "RecurBQR Usage: " << maxHolderUsage << std::endl;
        REQUIRE(all_correct == true);
    };
}

TEST_CASE("Dense RecurGLB", "[dense][RecurGLB]") {
    using namespace db_interval_set;
    using namespace loomrv;

    auto params = GENERATE(table<std::string, int>({
        {"Dense10", 10}, {"Dense100", 10}, {"Discrete", 10}, 
        {"Dense10", 100}, {"Dense100", 100}, {"Discrete", 100}, 
        {"Dense10", 1000}, {"Dense100", 1000}, {"Discrete", 1000}, 
    }));

    std::string CONDENSATION;
    int TIMINGS;
    std::tie(CONDENSATION, TIMINGS) = params;

    std::string benchmarkName = CONDENSATION + " " + std::to_string(TIMINGS);

    SECTION("RecurGLB " + benchmarkName) {
        ptl_parser parser;
        auto monitor = createDenseMultiPropertyMonitor(1000);
        parser.parse_dense("historically(once[:" + std::to_string(TIMINGS) + "]{p})", monitor);
        finalize_monitor(monitor, {"p"});

        std::string file_name = "data/fullsuite/RecurGLB/" + CONDENSATION + "/1M/RecurGLB" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = binary_row_reader::readInputFile(file_name);
        
        bool all_correct = true;
        int maxHolderUsage = 0;

        for (int i = 1; i < allInputs.size(); i++){
            auto outputs = eval_multi_property(monitor, allInputs[i - 1].time, allInputs[i].time, {allInputs[i - 1].p});
            
            if (toVectorIntervals(outputs[0]) != std::vector<Interval>{{allInputs[i - 1].time, allInputs[i].time}}) {
                all_correct = false;
            }
            maxHolderUsage = std::max(maxHolderUsage, monitor.holder.writeIndex);
            
        }
        std::cout << "RecurGLB Usage: " << maxHolderUsage << std::endl;
        REQUIRE(all_correct == true);
    };
}


TEST_CASE("Dense RespondBQR", "[dense][RespondBQR]") {
    using namespace db_interval_set;
    using namespace loomrv;

    auto params = GENERATE(table<std::string, int>({
        {"Dense10", 10}, {"Dense100", 10}, {"Discrete", 10}, 
        {"Dense10", 100}, {"Dense100", 100}, {"Discrete", 100}, 
        {"Dense10", 1000}, {"Dense100", 1000}, {"Discrete", 1000}, 
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
        auto monitor = createDenseMultiPropertyMonitor(1000);
        parser.parse_dense("historically(({r} && !{q} && once{q}) -> ( (({s} -> once[" + std::to_string(once_a) + ":" + std::to_string(once_b) + "]{p}) and not((not {s}) since[" + std::to_string(since_a) + ":] {p})) since {q}))", monitor);
        finalize_monitor(monitor, {"q", "p", "s", "r"});

        std::string file_name = "data/fullsuite/RespondBQR/" + CONDENSATION + "/1M/RespondBQR" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = binary_row_reader::readInputFile(file_name);
        
        bool all_correct = true;
        int maxHolderUsage = 0;

        for (int i = 1; i < allInputs.size(); i++){
            auto outputs = eval_multi_property(monitor, allInputs[i - 1].time, allInputs[i].time, {allInputs[i - 1].q, allInputs[i - 1].p, allInputs[i - 1].s, allInputs[i - 1].r});
            
            if (toVectorIntervals(outputs[0]) != std::vector<Interval>{{allInputs[i - 1].time, allInputs[i].time}}) {
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

TEST_CASE("Dense RespondGLB", "[dense][RespondGLB]") {
    using namespace db_interval_set;
    using namespace loomrv;

    auto params = GENERATE(table<std::string, int>({
        {"Dense10", 10}, {"Dense100", 10}, {"Discrete", 10}, 
        {"Dense10", 100}, {"Dense100", 100}, {"Discrete", 100}, 
        {"Dense10", 1000}, {"Dense100", 1000}, {"Discrete", 1000}, 
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
        auto monitor = createDenseMultiPropertyMonitor(1000);
        parser.parse_dense("historically(({s} -> once[" + std::to_string(once_a) + ":" + std::to_string(once_b) + "]{p}) and not((not {s}) since[" + std::to_string(since_a) + ":] {p}))", monitor);
        finalize_monitor(monitor, {"p", "s"});

        std::string file_name = "data/fullsuite/RespondGLB/" + CONDENSATION + "/1M/RespondGLB" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = binary_row_reader::readInputFile(file_name);
        
        bool all_correct = true;
        int maxHolderUsage = 0;

        for (int i = 1; i < allInputs.size(); i++){
            auto outputs = eval_multi_property(monitor, allInputs[i - 1].time, allInputs[i].time, {allInputs[i - 1].p, allInputs[i - 1].s});
            
            if (toVectorIntervals(outputs[0]) != std::vector<Interval>{{allInputs[i - 1].time, allInputs[i].time}}) {
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