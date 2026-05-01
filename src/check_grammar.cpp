#include <iostream>
#include <string>
#include <loomrv/ptl.hpp>
#include <loomrv/MTLEngine.hpp>

using namespace loomrv;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " \"<formula>\"" << std::endl;
        return 1;
    }

    std::string formula = argv[1];
    
    ptl_parser parser;
    try {
        auto monitor = createDiscreteMultiPropertyMonitor(1000);
        parser.parse_discrete(formula, monitor);
        std::cout << "Formula is valid!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Syntax Error: " << e.what() << std::endl;
        return 1;
    }
}
