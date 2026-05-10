#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <loomrv/ptl.hpp>

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
    int formula_num = 0;
    int total_hits = 0;
    bool any_hits = false;

    while (std::getline(file, formula)) {
        if (formula.empty()) continue;
        formula_num++;

        ptl_parser parser;
        try {
            auto nodes = parser.parse(formula);
            int hits = parser.dedup_hits;
            int node_count = static_cast<int>(nodes.size());
            int raw_count = node_count + hits;  // total calls = created + reused
            
            if (hits > 0) {
                any_hits = true;
                std::cout << "Formula " << formula_num << ": "
                          << node_count << " nodes (deduped), "
                          << raw_count << " raw nodes, "
                          << hits << " DEDUP HITS  <-- !!!"
                          << std::endl;
                std::cout << "  " << formula << std::endl;
            } else {
                std::cout << "Formula " << formula_num << ": "
                          << node_count << " nodes, 0 dedup hits"
                          << std::endl;
            }
            total_hits += hits;
        } catch (const std::exception& e) {
            std::cerr << "Error parsing formula " << formula_num << ": "
                      << e.what() << std::endl;
        }
    }

    std::cout << "\n=== Summary ===" << std::endl;
    std::cout << "Total formulas: " << formula_num << std::endl;
    std::cout << "Total dedup hits across all individual formulas: " << total_hits << std::endl;
    if (any_hits) {
        std::cout << "WARNING: Intra-formula dedup IS firing!" << std::endl;
    } else {
        std::cout << "CONFIRMED: No intra-formula dedup occurs for any formula." << std::endl;
    }

    return 0;
}
