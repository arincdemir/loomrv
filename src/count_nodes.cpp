#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <loomrv/ptl.hpp>
#include <loomrv/MTLEngine.hpp>

using namespace loomrv;

namespace {

constexpr std::array<std::pair<NodeType, const char *>, 9> NODE_TYPES{{
    {NodeType::PROPOSITION, "proposition"},
    {NodeType::AND, "and"},
    {NodeType::OR, "or"},
    {NodeType::NOT, "not"},
    {NodeType::IMPLIES, "implies"},
    {NodeType::EVENTUALLY, "once"},
    {NodeType::ALWAYS, "historically"},
    {NodeType::SINCE, "since"},
    {NodeType::TEST, "test"},
}};

void print_usage(const char *program) {
    std::cerr << "Usage: " << program << " [--json] <formulas.txt>" << std::endl;
}

} // namespace

int main(int argc, char** argv) {
    bool json_output = false;
    std::string filename;

    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--json") {
            json_output = true;
        } else if (!argument.empty() && argument.front() == '-') {
            std::cerr << "Unknown option: " << argument << std::endl;
            print_usage(argv[0]);
            return 1;
        } else if (filename.empty()) {
            filename = argument;
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }

    if (filename.empty()) {
        print_usage(argv[0]);
        return 1;
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << filename << std::endl;
        return 1;
    }

    std::string formula;
    std::size_t property_count = 0;
    std::size_t sequential_nodes = 0;
    
    ptl_parser multi_parser;
    auto multi_monitor = createDiscreteMultiPropertyMonitor(1000);

    while (std::getline(file, formula)) {
        if (formula.empty()) continue;
        ++property_count;
        
        try {
            // Count nodes for this single formula with a fresh monitor
            ptl_parser single_parser;
            auto single_monitor = createDiscreteMultiPropertyMonitor(1000);
            single_parser.parse_discrete(formula, single_monitor);
            sequential_nodes += single_monitor.nodes.size();

            // Accumulate in multi-property monitor
            multi_parser.parse_discrete(formula, multi_monitor);
        } catch (const std::exception& e) {
            std::cerr << "Error parsing formula: " << formula << "\n" << e.what() << std::endl;
            return 1;
        }
    }
    
    const std::size_t multi_nodes = multi_monitor.nodes.size();
    const std::size_t eliminated_nodes = sequential_nodes - multi_nodes;

    std::array<std::size_t, NODE_TYPES.size()> node_type_counts{};
    for (const auto &node : multi_monitor.nodes) {
        for (std::size_t i = 0; i < NODE_TYPES.size(); ++i) {
            if (node.type == NODE_TYPES[i].first) {
                ++node_type_counts[i];
                break;
            }
        }
    }

    if (json_output) {
        std::cout << "{\n"
                  << "  \"schema_version\": 1,\n"
                  << "  \"property_count\": " << property_count << ",\n"
                  << "  \"independent_node_count\": " << sequential_nodes << ",\n"
                  << "  \"shared_dag_node_count\": " << multi_nodes << ",\n"
                  << "  \"eliminated_node_count\": " << eliminated_nodes << ",\n"
                  << "  \"compression_ratio\": ";
        if (multi_nodes == 0) {
            std::cout << "null";
        } else {
            std::cout << std::setprecision(15)
                      << static_cast<double>(sequential_nodes) / multi_nodes;
        }
        std::cout << ",\n"
                  << "  \"property_root_count\": "
                  << multi_monitor.propertyRootNodeIndexes.size() << ",\n"
                  << "  \"node_type_counts\": {\n";
        for (std::size_t i = 0; i < NODE_TYPES.size(); ++i) {
            std::cout << "    \"" << NODE_TYPES[i].second << "\": "
                      << node_type_counts[i]
                      << (i + 1 == NODE_TYPES.size() ? "\n" : ",\n");
        }
        std::cout << "  }\n"
                  << "}\n";
        return 0;
    }

    std::cout << "=== Node Count Report: " << filename << " ===" << std::endl;
    std::cout << "Sequential AST Nodes (Sum) : " << sequential_nodes << std::endl;
    std::cout << "Deduplicated AST Nodes     : " << multi_nodes << std::endl;
    
    if (sequential_nodes > 0) {
        double reduction_pct = 100.0 * eliminated_nodes / sequential_nodes;
        double compression_factor = (double)sequential_nodes / multi_nodes;
        std::cout << "Reduction                  : " << eliminated_nodes
                  << " nodes (" << reduction_pct << "%)" << std::endl;
        std::cout << "AST Size Compression       : " << compression_factor << "x smaller" << std::endl;
    }

    return 0;
}
