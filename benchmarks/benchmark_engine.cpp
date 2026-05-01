#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_all.hpp>
#include <fstream>
#include <iostream>

#include "loomrv/interval_set.hpp"
#include "loomrv/MTLEngine.hpp"
#include "loomrv/binary_row_reader.hpp"
// wallgrind memory leak memcheck
//scanbuild

class InputCache {
public:
    static const std::vector<binary_row_reader::TimescalesInput>& get(const std::string& filename) {
        static std::vector<binary_row_reader::TimescalesInput> cache;
        static std::string cachedFilename;

        // If we are asking for a new file, clear the old one and load the new one
        if (cachedFilename != filename) {
            // Forcefully free memory from the previous file to the OS
            std::vector<binary_row_reader::TimescalesInput>().swap(cache); 
            
            // Load new file
            cache = binary_row_reader::readInputFile(filename);
            cachedFilename = filename;
        }
        return cache;
    }
};


TEST_CASE("Dense AbsentAQ", "[dense_benchmarks][AbsentAQ]") {

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

    BENCHMARK_ADVANCED("AbsentAQ " + benchmarkName)(Catch::Benchmark::Chronometer meter) {
        IntervalSetHolder holder = newHolder(1000);
        DenseNode q{empty(holder), empty(holder), NodeType::PROPOSITION, 0, 0, 0, 0};
        DenseNode p{empty(holder), empty(holder), NodeType::PROPOSITION, 1, 0, 0, 0};
        DenseNode once{empty(holder), empty(holder), NodeType::EVENTUALLY, 0, 0, 0, TIMINGS};
        DenseNode notNode{empty(holder), empty(holder), NodeType::NOT, 0, 1, 0, 0};
        DenseNode since{empty(holder), empty(holder), NodeType::SINCE, 3, 0, 0, B_INFINITY};
        DenseNode implies{empty(holder), empty(holder), NodeType::IMPLIES, 2, 4, 0, 0};
        DenseNode always{empty(holder), empty(holder), NodeType::ALWAYS, 0, 5, 0, B_INFINITY};
        std::vector<DenseNode> nodes{q, p, once, notNode, since, implies, always};

        std::string file_name = "data/fullsuite/AbsentAQ/" + CONDENSATION + "/1M/AbsentAQ" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = InputCache::get(file_name);        
        meter.measure([&] {  
            IntervalSet finalOutput;
            for (int i = 1; i < allInputs.size(); i++){
                auto newInput = allInputs[i];
                IntervalSet output = run_evaluation(nodes, holder, allInputs[i - 1].time, allInputs[i].time, {allInputs[i - 1].q, allInputs[i - 1].p});
                finalOutput = output;
                swapBuffers(holder);
            }
            //std::cout << toVectorIntervals << std::endl;
            return finalOutput;
        });
        destroyHolder(holder);

    };
}


TEST_CASE("Dense AbsentBQR", "[dense_benchmarks][AbsentBQR]") {

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

    std::string CONDENSATION;
    int TIMINGS;
    std::tie(CONDENSATION, TIMINGS) = params;

    std::string benchmarkName = CONDENSATION + " " + std::to_string(TIMINGS);

    BENCHMARK_ADVANCED("AbsentBQR " + benchmarkName)(Catch::Benchmark::Chronometer meter) {
        IntervalSetHolder holder = newHolder(1000);

        const int since_a = 3 * (TIMINGS / 10);
        const int since_b = TIMINGS;

        DenseNode q{empty(holder), empty(holder), NodeType::PROPOSITION, 0, 0, 0, 0};      // 0
        DenseNode p{empty(holder), empty(holder), NodeType::PROPOSITION, 1, 0, 0, 0};      // 1
        DenseNode r{empty(holder), empty(holder), NodeType::PROPOSITION, 2, 0, 0, 0};      // 2
        DenseNode not_q{empty(holder), empty(holder), NodeType::NOT, 0, 0, 0, 0};          // 3
        DenseNode once_q{empty(holder), empty(holder), NodeType::EVENTUALLY, 0, 0, 0, B_INFINITY}; // 4
        DenseNode and1{empty(holder), empty(holder), NodeType::AND, 2, 3, 0, 0};           // 5
        DenseNode and2{empty(holder), empty(holder), NodeType::AND, 5, 4, 0, 0};           // 6
        DenseNode not_p{empty(holder), empty(holder), NodeType::NOT, 0, 1, 0, 0};          // 7
        DenseNode since_node{empty(holder), empty(holder), NodeType::SINCE, 7, 0, since_a, since_b}; // 8
        DenseNode implies_node{empty(holder), empty(holder), NodeType::IMPLIES, 6, 8, 0, 0}; // 9
        DenseNode always_node{empty(holder), empty(holder), NodeType::ALWAYS, 0, 9, 0, B_INFINITY}; // 10

        std::vector<DenseNode> nodes{q, p, r, not_q, once_q, and1, and2, not_p, since_node, implies_node, always_node};

        std::string file_name = "data/fullsuite/AbsentBQR/" + CONDENSATION + "/1M/AbsentBQR" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = InputCache::get(file_name);
        meter.measure([&] { 
            IntervalSet finalOutput;
            for (int i = 1; i < allInputs.size(); i++){
                auto newInput = allInputs[i];
                // Order matches nodes: q, p, r
                IntervalSet output = run_evaluation(nodes, holder, allInputs[i - 1].time, allInputs[i].time, {allInputs[i - 1].q, allInputs[i - 1].p, allInputs[i - 1].r});
                finalOutput = output;
                swapBuffers(holder);
            }
            return finalOutput;
        });
        destroyHolder(holder);
    };
}


TEST_CASE("Dense AbsentBR", "[dense_benchmarks][AbsentBR]") {

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

    BENCHMARK_ADVANCED("AbsentBR " + benchmarkName)(Catch::Benchmark::Chronometer meter) {
        IntervalSetHolder holder = newHolder(1000);

        const int inner_always_b = TIMINGS;

        DenseNode q{empty(holder), empty(holder), NodeType::PROPOSITION, 0, 0, 0, 0};      // 0
        DenseNode p{empty(holder), empty(holder), NodeType::PROPOSITION, 1, 0, 0, 0};      // 1
        DenseNode r{empty(holder), empty(holder), NodeType::PROPOSITION, 2, 0, 0, 0};      // 2
        DenseNode not_p{empty(holder), empty(holder), NodeType::NOT, 0, 1, 0, 0};          // 3
        DenseNode inner_always{empty(holder), empty(holder), NodeType::ALWAYS, 0, 3, 0, inner_always_b}; // 4
        DenseNode implies_node{empty(holder), empty(holder), NodeType::IMPLIES, 2, 4, 0, 0}; // 5
        DenseNode root_always{empty(holder), empty(holder), NodeType::ALWAYS, 0, 5, 0, B_INFINITY}; // 6

        std::vector<DenseNode> nodes{q, p, r, not_p, inner_always, implies_node, root_always};

        std::string file_name = "data/fullsuite/AbsentBR/" + CONDENSATION + "/1M/AbsentBR" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = InputCache::get(file_name);
        meter.measure([&] { 
            IntervalSet finalOutput;
            for (int i = 1; i < allInputs.size(); i++){
                auto newInput = allInputs[i];
                // Order matches nodes: q, p, r
                IntervalSet output = run_evaluation(nodes, holder, allInputs[i - 1].time, allInputs[i].time, {allInputs[i - 1].q, allInputs[i - 1].p, allInputs[i - 1].r});
                finalOutput = output;
                swapBuffers(holder);
            }
            return finalOutput;
        });
        destroyHolder(holder);
    };
}


TEST_CASE("Dense AlwaysAQ", "[dense_benchmarks][AlwaysAQ]") {

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

    BENCHMARK_ADVANCED("AlwaysAQ " + benchmarkName)(Catch::Benchmark::Chronometer meter) {
        IntervalSetHolder holder = newHolder(1000);
        
        const int once_b = TIMINGS;

        DenseNode q{empty(holder), empty(holder), NodeType::PROPOSITION, 0, 0, 0, 0};      // 0
        DenseNode p{empty(holder), empty(holder), NodeType::PROPOSITION, 1, 0, 0, 0};      // 1
        DenseNode r{empty(holder), empty(holder), NodeType::PROPOSITION, 2, 0, 0, 0};      // 2
        DenseNode once_q{empty(holder), empty(holder), NodeType::EVENTUALLY, 0, 0, 0, once_b}; // 3
        DenseNode since_node{empty(holder), empty(holder), NodeType::SINCE, 1, 0, 0, B_INFINITY}; // 4
        DenseNode implies_node{empty(holder), empty(holder), NodeType::IMPLIES, 3, 4, 0, 0}; // 5
        DenseNode root_always{empty(holder), empty(holder), NodeType::ALWAYS, 0, 5, 0, B_INFINITY}; // 6

        std::vector<DenseNode> nodes{q, p, r, once_q, since_node, implies_node, root_always};

        std::string file_name = "data/fullsuite/AlwaysAQ/" + CONDENSATION + "/1M/AlwaysAQ" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = InputCache::get(file_name);
        meter.measure([&] { 
            IntervalSet finalOutput;
            for (int i = 1; i < allInputs.size(); i++){
                auto newInput = allInputs[i];
                // Order matches nodes: q, p, r
                IntervalSet output = run_evaluation(nodes, holder, allInputs[i - 1].time, allInputs[i].time, {allInputs[i - 1].q, allInputs[i - 1].p, allInputs[i - 1].r});
                finalOutput = output;
                swapBuffers(holder);
            }
            return finalOutput;
        });
        destroyHolder(holder);
    };
}


TEST_CASE("Dense AlwaysBQR", "[dense_benchmarks][AlwaysBQR]") {

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

    BENCHMARK_ADVANCED("AlwaysBQR " + benchmarkName)(Catch::Benchmark::Chronometer meter) {
        IntervalSetHolder holder = newHolder(1000);

        const int since_a = 3 * (TIMINGS / 10);
        const int since_b = TIMINGS;

        DenseNode q{empty(holder), empty(holder), NodeType::PROPOSITION, 0, 0, 0, 0};      // 0
        DenseNode p{empty(holder), empty(holder), NodeType::PROPOSITION, 1, 0, 0, 0};      // 1
        DenseNode r{empty(holder), empty(holder), NodeType::PROPOSITION, 2, 0, 0, 0};      // 2
        DenseNode not_q{empty(holder), empty(holder), NodeType::NOT, 0, 0, 0, 0};          // 3
        DenseNode once_q{empty(holder), empty(holder), NodeType::EVENTUALLY, 0, 0, 0, B_INFINITY}; // 4
        DenseNode and1{empty(holder), empty(holder), NodeType::AND, 2, 3, 0, 0};           // 5
        DenseNode and2{empty(holder), empty(holder), NodeType::AND, 5, 4, 0, 0};           // 6
        DenseNode not_p{empty(holder), empty(holder), NodeType::NOT, 0, 1, 0, 0};          // 7
        DenseNode since_node{empty(holder), empty(holder), NodeType::SINCE, 1, 0, since_a, since_b}; // 8
        DenseNode implies_node{empty(holder), empty(holder), NodeType::IMPLIES, 6, 8, 0, 0}; // 9
        DenseNode always_node{empty(holder), empty(holder), NodeType::ALWAYS, 0, 9, 0, B_INFINITY}; // 10

        std::vector<DenseNode> nodes{q, p, r, not_q, once_q, and1, and2, not_p, since_node, implies_node, always_node};

        std::string file_name = "data/fullsuite/AlwaysBQR/" + CONDENSATION + "/1M/AlwaysBQR" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = InputCache::get(file_name);
        meter.measure([&] { 
            IntervalSet finalOutput;
            for (int i = 1; i < allInputs.size(); i++){
                auto newInput = allInputs[i];
                // Order matches nodes: q, p, r
                IntervalSet output = run_evaluation(nodes, holder, allInputs[i - 1].time, allInputs[i].time, {allInputs[i - 1].q, allInputs[i - 1].p, allInputs[i - 1].r});
                finalOutput = output;
                swapBuffers(holder);
            }
            return finalOutput;
        });
        destroyHolder(holder);
    };
}


TEST_CASE("Dense AlwaysBR", "[dense_benchmarks][AlwaysBR]") {

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

    BENCHMARK_ADVANCED("AlwaysBR " + benchmarkName)(Catch::Benchmark::Chronometer meter) {
        IntervalSetHolder holder = newHolder(1000);

        const int inner_always_b = TIMINGS;

        DenseNode q{empty(holder), empty(holder), NodeType::PROPOSITION, 0, 0, 0, 0};      // 0
        DenseNode p{empty(holder), empty(holder), NodeType::PROPOSITION, 1, 0, 0, 0};      // 1
        DenseNode r{empty(holder), empty(holder), NodeType::PROPOSITION, 2, 0, 0, 0};      // 2
        DenseNode inner_always{empty(holder), empty(holder), NodeType::ALWAYS, 0, 1, 0, inner_always_b}; // 3
        DenseNode implies_node{empty(holder), empty(holder), NodeType::IMPLIES, 2, 3, 0, 0}; // 4
        DenseNode root_always{empty(holder), empty(holder), NodeType::ALWAYS, 0, 4, 0, B_INFINITY}; // 5

        std::vector<DenseNode> nodes{q, p, r, inner_always, implies_node, root_always};

        std::string file_name = "data/fullsuite/AlwaysBR/" + CONDENSATION + "/1M/AlwaysBR" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = InputCache::get(file_name);
        meter.measure([&] { 
            IntervalSet finalOutput;
            for (int i = 1; i < allInputs.size(); i++){
                auto newInput = allInputs[i];
                // Order matches nodes: q, p, r
                IntervalSet output = run_evaluation(nodes, holder, allInputs[i - 1].time, allInputs[i].time, {allInputs[i - 1].q, allInputs[i - 1].p, allInputs[i - 1].r});
                finalOutput = output;
                swapBuffers(holder);
            }
            return finalOutput;
        });
        destroyHolder(holder);
    };
}


TEST_CASE("Dense RecurBQR", "[dense_benchmarks][RecurBQR]") {

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

    BENCHMARK_ADVANCED("RecurBQR " + benchmarkName)(Catch::Benchmark::Chronometer meter) {
        IntervalSetHolder holder = newHolder(1000);

        const int inner_once_b = TIMINGS;

        DenseNode q{empty(holder), empty(holder), NodeType::PROPOSITION, 0, 0, 0, 0};      // 0
        DenseNode p{empty(holder), empty(holder), NodeType::PROPOSITION, 1, 0, 0, 0};      // 1
        DenseNode r{empty(holder), empty(holder), NodeType::PROPOSITION, 2, 0, 0, 0};      // 2
        DenseNode not_q{empty(holder), empty(holder), NodeType::NOT, 0, 0, 0, 0};          // 3
        DenseNode once_q{empty(holder), empty(holder), NodeType::EVENTUALLY, 0, 0, 0, B_INFINITY}; // 4
        DenseNode and1{empty(holder), empty(holder), NodeType::AND, 2, 3, 0, 0};           // 5
        DenseNode and2{empty(holder), empty(holder), NodeType::AND, 5, 4, 0, 0};           // 6
        DenseNode p_or_q{empty(holder), empty(holder), NodeType::OR, 1, 0, 0, 0};          // 7
        DenseNode once_p_or_q{empty(holder), empty(holder), NodeType::EVENTUALLY, 0, 7, 0, inner_once_b}; // 8
        DenseNode since_node{empty(holder), empty(holder), NodeType::SINCE, 8, 0, 0, B_INFINITY}; // 9
        DenseNode implies_node{empty(holder), empty(holder), NodeType::IMPLIES, 6, 9, 0, 0}; // 10
        DenseNode root_always{empty(holder), empty(holder), NodeType::ALWAYS, 0, 10, 0, B_INFINITY}; // 11

        std::vector<DenseNode> nodes{q, p, r, not_q, once_q, and1, and2, p_or_q, once_p_or_q, since_node, implies_node, root_always};

        std::string file_name = "data/fullsuite/RecurBQR/" + CONDENSATION + "/1M/RecurBQR" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = InputCache::get(file_name);
        meter.measure([&] { 
            IntervalSet finalOutput;
            for (int i = 1; i < allInputs.size(); i++){
                auto newInput = allInputs[i];
                // Order matches nodes: q, p, r
                IntervalSet output = run_evaluation(nodes, holder, allInputs[i - 1].time, allInputs[i].time, {allInputs[i - 1].q, allInputs[i - 1].p, allInputs[i - 1].r});
                finalOutput = output;
                swapBuffers(holder);
            }
            return finalOutput;
        });
        destroyHolder(holder);

    };

}


TEST_CASE("Dense RecurGLB", "[dense_benchmarks][RecurGLB]") {

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

    BENCHMARK_ADVANCED("RecurGLB " + benchmarkName)(Catch::Benchmark::Chronometer meter) {
        IntervalSetHolder holder = newHolder(1000);
        DenseNode p{empty(holder), empty(holder), NodeType::PROPOSITION, 0, 0, 0, 0};
        DenseNode once{empty(holder), empty(holder), NodeType::EVENTUALLY, 0, 0, 0, TIMINGS};
        DenseNode always{empty(holder), empty(holder), NodeType::ALWAYS, 0, 1, 0, B_INFINITY};
        std::vector<DenseNode> nodes{p, once, always};

        std::string file_name = "data/fullsuite/RecurGLB/" + CONDENSATION + "/1M/RecurGLB" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = InputCache::get(file_name);
        meter.measure([&] {  
            IntervalSet finalOutput;
            for (int i = 1; i < allInputs.size(); i++){
                auto newInput = allInputs[i];
                // Order matches nodes: just p
                IntervalSet output = run_evaluation(nodes, holder, allInputs[i - 1].time, allInputs[i].time, {allInputs[i - 1].p});
                finalOutput = output;
                swapBuffers(holder);
            }
            return finalOutput;
        });
        destroyHolder(holder);

    };

}


TEST_CASE("Dense RespondBQR", "[dense_benchmarks][RespondBQR]") {

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

    BENCHMARK_ADVANCED("RespondBQR " + benchmarkName)(Catch::Benchmark::Chronometer meter) {
        IntervalSetHolder holder = newHolder(1000);

        const int once_a = 3 * (TIMINGS / 10);
        const int once_b = TIMINGS;
        const int since_a = TIMINGS;

        DenseNode q{empty(holder), empty(holder), NodeType::PROPOSITION, 0, 0, 0, 0};      // 0
        DenseNode p{empty(holder), empty(holder), NodeType::PROPOSITION, 1, 0, 0, 0};      // 1
        DenseNode s{empty(holder), empty(holder), NodeType::PROPOSITION, 2, 0, 0, 0};      // 2
        DenseNode r{empty(holder), empty(holder), NodeType::PROPOSITION, 3, 0, 0, 0};      // 3
        DenseNode not_q{empty(holder), empty(holder), NodeType::NOT, 0, 0, 0, 0};          // 4
        DenseNode once_q{empty(holder), empty(holder), NodeType::EVENTUALLY, 0, 0, 0, B_INFINITY}; // 5
        DenseNode and_A1{empty(holder), empty(holder), NodeType::AND, 3, 4, 0, 0};         // 6
        DenseNode and_A2{empty(holder), empty(holder), NodeType::AND, 6, 5, 0, 0};         // 7
        DenseNode once_p{empty(holder), empty(holder), NodeType::EVENTUALLY, 0, 1, once_a, once_b}; // 8
        DenseNode implies_D{empty(holder), empty(holder), NodeType::IMPLIES, 2, 8, 0, 0};    // 9
        DenseNode not_s{empty(holder), empty(holder), NodeType::NOT, 0, 2, 0, 0};          // 10
        DenseNode since_F{empty(holder), empty(holder), NodeType::SINCE, 10, 1, since_a, B_INFINITY}; // 11
        DenseNode not_F{empty(holder), empty(holder), NodeType::NOT, 0, 11, 0, 0};         // 12
        DenseNode and_C{empty(holder), empty(holder), NodeType::AND, 9, 12, 0, 0};         // 13
        DenseNode since_B{empty(holder), empty(holder), NodeType::SINCE, 13, 0, 0, B_INFINITY}; // 14
        DenseNode implies_main{empty(holder), empty(holder), NodeType::IMPLIES, 7, 14, 0, 0}; // 15
        DenseNode root_always{empty(holder), empty(holder), NodeType::ALWAYS, 0, 15, 0, B_INFINITY}; // 16

        std::vector<DenseNode> nodes{q, p, s, r, not_q, once_q, and_A1, and_A2, once_p, implies_D, not_s, since_F, not_F, and_C, since_B, implies_main, root_always};

        std::string file_name = "data/fullsuite/RespondBQR/" + CONDENSATION + "/1M/RespondBQR" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = InputCache::get(file_name);
        meter.measure([&] { 
            IntervalSet finalOutput;
            for (int i = 1; i < allInputs.size(); i++){
                auto newInput = allInputs[i];
                // Order matches nodes: q, p, s, r
                IntervalSet output = run_evaluation(nodes, holder, allInputs[i - 1].time, allInputs[i].time, {allInputs[i - 1].q, allInputs[i - 1].p, allInputs[i - 1].s, allInputs[i - 1].r});
                finalOutput = output;
                swapBuffers(holder);
            }
            return finalOutput;
        });
        destroyHolder(holder);

    };
}


TEST_CASE("Dense RespondGLB", "[dense_benchmarks][RespondGLB]") {

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

    BENCHMARK_ADVANCED("RespondGLB " + benchmarkName)(Catch::Benchmark::Chronometer meter) {
        IntervalSetHolder holder = newHolder(1000);

        const int once_a = 3 * (TIMINGS / 10);
        const int once_b = TIMINGS;
        const int since_a = TIMINGS;

        DenseNode p{empty(holder), empty(holder), NodeType::PROPOSITION, 0, 0, 0, 0};      // 0
        DenseNode s{empty(holder), empty(holder), NodeType::PROPOSITION, 1, 0, 0, 0};      // 1
        DenseNode once_p{empty(holder), empty(holder), NodeType::EVENTUALLY, 0, 0, once_a, once_b}; // 2
        DenseNode implies_D{empty(holder), empty(holder), NodeType::IMPLIES, 1, 2, 0, 0};    // 3
        DenseNode not_s{empty(holder), empty(holder), NodeType::NOT, 0, 1, 0, 0};          // 4
        DenseNode since_F{empty(holder), empty(holder), NodeType::SINCE, 4, 0, since_a, B_INFINITY}; // 5
        DenseNode not_F{empty(holder), empty(holder), NodeType::NOT, 0, 5, 0, 0};         // 6
        DenseNode and_C{empty(holder), empty(holder), NodeType::AND, 3, 6, 0, 0};         // 7
        DenseNode root_always{empty(holder), empty(holder), NodeType::ALWAYS, 0, 7, 0, B_INFINITY}; // 8

        std::vector<DenseNode> nodes{p, s, once_p, implies_D, not_s, since_F, not_F, and_C, root_always};

        std::string file_name = "data/fullsuite/RespondGLB/" + CONDENSATION + "/1M/RespondGLB" + std::to_string(TIMINGS) +".row.bin";
        const auto& allInputs = InputCache::get(file_name);
        meter.measure([&] { 
            IntervalSet finalOutput;
            for (int i = 1; i < allInputs.size(); i++){
                // Order matches nodes: p, s
                IntervalSet output = run_evaluation(nodes, holder, allInputs[i - 1].time, allInputs[i].time, {allInputs[i - 1].p, allInputs[i - 1].s});
                finalOutput = output;
                swapBuffers(holder);
            }
            return finalOutput;
        });
        destroyHolder(holder);
    };
}