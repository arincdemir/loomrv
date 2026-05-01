#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <getopt.h>
#include <cstdlib>

#include "loomrv/MTLEngine.hpp"
#include "loomrv/ptl.hpp"
#include "loomrv/json_feeder.hpp"
#include "loomrv/binary_feeder.hpp"
#include "loomrv/interval_set.hpp"

using namespace db_interval_set;
using namespace loomrv;

// option keys
enum RYBINX_OPTS : uint8_t
{
    OPT_DENSE    = 'v',
    OPT_DISCRETE = 'x',
    OPT_BINARY   = 'b',
};

struct arguments
{
    char *trace_file = nullptr;
    char *properties_file = nullptr;
    bool dense    = false;
    bool discrete = false;
    bool binary   = false;
};

static void print_usage(const char *prog)
{
    std::fprintf(stderr,
        "Usage: %s [OPTION...] TRACE_FILE PROPERTIES_FILE\n"
        "LoomRV (Reelay) CLI Tool -- loomrv 0.2.0\n\n"
        "  -v, --dense      Use dense time model (default)\n"
        "  -x, --discrete   Use discrete time model\n"
        "  -b, --binary     Read trace from binary .row.bin format instead of NDJSON\n"
        "\nReport bugs to: Arinc Demir <github.com/arincdemir>\n",
        prog);
}

void discrete_case(const arguments &args);
void dense_case(const arguments &args);
void discrete_binary_case(const arguments &args);
void dense_binary_case(const arguments &args);

int main(int argc, char **argv)
{
    struct arguments args;

    static struct option long_options[] = {
        {"dense",    no_argument, nullptr, OPT_DENSE},
        {"discrete", no_argument, nullptr, OPT_DISCRETE},
        {"binary",   no_argument, nullptr, OPT_BINARY},
        {nullptr, 0, nullptr, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "vxb", long_options, nullptr)) != -1)
    {
        switch (opt)
        {
        case OPT_DENSE:
            args.dense = true;
            break;
        case OPT_DISCRETE:
            args.discrete = true;
            break;
        case OPT_BINARY:
            args.binary = true;
            break;
        default:
            print_usage(argv[0]);
            return 1;
        }
    }

    if (optind + 2 > argc)
    {
        print_usage(argv[0]);
        return 1;
    }
    args.trace_file      = argv[optind];
    args.properties_file = argv[optind + 1];

    if (!args.dense && !args.discrete)
    {
        args.dense = true; // Default to dense
    }

    if (args.binary)
    {
        if (args.discrete)
            discrete_binary_case(args);
        else
            dense_binary_case(args);
    }
    else
    {
        if (args.discrete)
            discrete_case(args);
        else
            dense_case(args);
    }

    return 0;
}

void dense_case(const arguments &args)
{
    std::ifstream props_in(args.properties_file);
    if (!props_in.is_open())
    {
        std::cerr << "Error opening properties file: " << args.properties_file << std::endl;
        exit(1);
    }

    std::vector<std::string> formulas;
    std::string line;
    while (std::getline(props_in, line))
    {
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) continue;
        formulas.push_back(line);
    }

    if (formulas.empty())
    {
        std::cerr << "No formulas found in properties file." << std::endl;
        exit(1);
    }

    DenseMultiPropertyMonitor monitor = createDenseMultiPropertyMonitor(3000);
    ptl_parser parser;

    for (const auto &f : formulas)
    {
        try {
            parser.parse_dense(f, monitor);
        } catch (const std::exception &e) {
            std::cerr << "Error parsing formula: " << f << "\n" << e.what() << std::endl;
            exit(1);
        }
    }
    finalize_monitor(monitor);

    auto *feeder = create_dense_json_feeder(monitor, args.trace_file);
    if (!feeder)
    {
        std::cerr << "Error creating feeder for trace file: " << args.trace_file << std::endl;
        exit(1);
    }

    while (feed_next(feeder))
    {
    }

    destroy_feeder(feeder);
}

void dense_binary_case(const arguments &args)
{
    std::ifstream props_in(args.properties_file);
    if (!props_in.is_open())
    {
        std::cerr << "Error opening properties file: " << args.properties_file << std::endl;
        exit(1);
    }

    std::vector<std::string> formulas;
    std::string line;
    while (std::getline(props_in, line))
    {
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) continue;
        formulas.push_back(line);
    }

    if (formulas.empty())
    {
        std::cerr << "No formulas found in properties file." << std::endl;
        exit(1);
    }

    DenseMultiPropertyMonitor monitor = createDenseMultiPropertyMonitor(3000);
    ptl_parser parser;

    for (const auto &f : formulas)
    {
        try {
            parser.parse_dense(f, monitor);
        } catch (const std::exception &e) {
            std::cerr << "Error parsing formula: " << f << "\n" << e.what() << std::endl;
            exit(1);
        }
    }
    // Finalization (with p,q,r,s ordering) is handled by the binary feeder.

    auto *feeder = create_dense_binary_feeder(monitor, args.trace_file);
    if (!feeder)
    {
        std::cerr << "Error creating binary feeder for trace file: " << args.trace_file << std::endl;
        exit(1);
    }

    while (feed_next(feeder))
    {
    }

    destroy_feeder(feeder);
}

void discrete_case(const arguments &args)
{
    std::ifstream props_in(args.properties_file);
    if (!props_in.is_open())
    {
        std::cerr << "Error opening properties file: " << args.properties_file << std::endl;
        exit(1);
    }

    std::vector<std::string> formulas;
    std::string line;
    while (std::getline(props_in, line))
    {
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) continue;
        formulas.push_back(line);
    }

    if (formulas.empty())
    {
        std::cerr << "No formulas found in properties file." << std::endl;
        exit(1);
    }

    DiscreteMultiPropertyMonitor monitor = createDiscreteMultiPropertyMonitor(3000);
    ptl_parser parser;

    for (const auto &f : formulas)
    {
        try {
            parser.parse_discrete(f, monitor);
        } catch (const std::exception &e) {
            std::cerr << "Error parsing formula: " << f << "\n" << e.what() << std::endl;
            exit(1);
        }
    }
    finalize_monitor(monitor);

    auto *feeder = create_discrete_json_feeder(monitor, args.trace_file);
    if (!feeder)
    {
        std::cerr << "Error creating feeder for trace file: " << args.trace_file << std::endl;
        exit(1);
    }

    while (feed_next(feeder))
    {
    }

    destroy_feeder(feeder);
}

void discrete_binary_case(const arguments &args)
{
    std::ifstream props_in(args.properties_file);
    if (!props_in.is_open())
    {
        std::cerr << "Error opening properties file: " << args.properties_file << std::endl;
        exit(1);
    }

    std::vector<std::string> formulas;
    std::string line;
    while (std::getline(props_in, line))
    {
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) continue;
        formulas.push_back(line);
    }

    if (formulas.empty())
    {
        std::cerr << "No formulas found in properties file." << std::endl;
        exit(1);
    }

    DiscreteMultiPropertyMonitor monitor = createDiscreteMultiPropertyMonitor(3000);
    ptl_parser parser;

    for (const auto &f : formulas)
    {
        try {
            parser.parse_discrete(f, monitor);
        } catch (const std::exception &e) {
            std::cerr << "Error parsing formula: " << f << "\n" << e.what() << std::endl;
            exit(1);
        }
    }
    // Finalization (with p,q,r,s ordering) is handled by the binary feeder.

    auto *feeder = create_discrete_binary_feeder(monitor, args.trace_file);
    if (!feeder)
    {
        std::cerr << "Error creating binary feeder for trace file: " << args.trace_file << std::endl;
        exit(1);
    }

    while (feed_next(feeder))
    {
    }

    destroy_feeder(feeder);
}
