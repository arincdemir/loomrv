#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <loomrv/ptl.hpp>
#include <loomrv/MTLEngine.hpp>

using namespace loomrv;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <formulas.txt>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << filename << std::endl;
        return 1;
    }

    std::string formula;
    int sequential_nodes = 0;
    int multi_nodes = 0;
    
    ptl_parser multi_parser;
    auto multi_monitor = createDiscreteMultiPropertyMonitor(1000);

    while (std::getline(file, formula)) {
        if (formula.empty()) continue;
        
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
    
    multi_nodes = multi_monitor.nodes.size();

    std::cout << "=== Node Count Report: " << filename << " ===" << std::endl;
    std::cout << "Sequential AST Nodes (Sum) : " << sequential_nodes << std::endl;
    std::cout << "Deduplicated AST Nodes     : " << multi_nodes << std::endl;
    
    if (sequential_nodes > 0) {
        double reduction_pct = 100.0 * (sequential_nodes - multi_nodes) / sequential_nodes;
        double compression_factor = (double)sequential_nodes / multi_nodes;
        std::cout << "Reduction                  : " << (sequential_nodes - multi_nodes) 
                  << " nodes (" << reduction_pct << "%)" << std::endl;
        std::cout << "AST Size Compression       : " << compression_factor << "x smaller" << std::endl;
    }

    return 0;
}
