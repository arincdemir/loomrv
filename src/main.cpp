#include <charconv>
#include <cstdlib>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <string>
#include <vector>

#include "loomrv/MTLEngine.hpp"
#include "loomrv/binary_feeder.hpp"
#include "loomrv/interval_set.hpp"
#include "loomrv/json_feeder.hpp"
#include "loomrv/ptl.hpp"

using namespace db_interval_set;
using namespace loomrv;

// option keys
enum RYBINX_OPTS : uint8_t {
  OPT_DENSE = 'v',
  OPT_DISCRETE = 'x',
  OPT_BINARY = 'b',
  OPT_PRINT = 'p',
  OPT_ARENA_CAPACITY = 'a',
};

struct arguments {
  char *trace_file = nullptr;
  char *properties_file = nullptr;
  bool dense = false;
  bool discrete = false;
  bool binary = false;
  bool print = false;
  unsigned int arena_capacity = 3000;
};

static void print_usage(const char *prog) {
  std::fprintf(stderr,
               "Usage: %s [OPTION...] TRACE_FILE PROPERTIES_FILE\n"
               "LoomRV (Reelay) CLI Tool -- loomrv 0.2.0\n\n"
               "  -v, --dense      Use dense time model (default)\n"
               "  -x, --discrete   Use discrete time model\n"
               "  -b, --binary     Read trace from binary .row.bin format "
               "instead of NDJSON\n"
               "  -p, --print      Print per-timestep verdicts to stdout\n"
               "  -a, --arena-capacity N\n"
               "                   Set each interval arena buffer's capacity "
               "(default: 3000)\n"
               "\nReport bugs to: Arinc Demir <github.com/arincdemir>\n",
               prog);
}

static bool parse_arena_capacity(const char *text, unsigned int &capacity) {
  if (text == nullptr || *text == '\0') {
    return false;
  }

  unsigned int value = 0;
  const char *end = text + std::char_traits<char>::length(text);
  const auto result = std::from_chars(text, end, value);
  if (result.ec != std::errc{} || result.ptr != end || value == 0) {
    return false;
  }

  capacity = value;
  return true;
}

void discrete_case(const arguments &args);
void dense_case(const arguments &args);
void discrete_binary_case(const arguments &args);
void dense_binary_case(const arguments &args);

int main(int argc, char **argv) {
  struct arguments args;

  static struct option long_options[] = {
      {"dense", no_argument, nullptr, OPT_DENSE},
      {"discrete", no_argument, nullptr, OPT_DISCRETE},
      {"binary", no_argument, nullptr, OPT_BINARY},
      {"print", no_argument, nullptr, OPT_PRINT},
      {"arena-capacity", required_argument, nullptr, OPT_ARENA_CAPACITY},
      {nullptr, 0, nullptr, 0}};

  opterr = 0;
  int opt;
  while ((opt = getopt_long(argc, argv, ":vxbpa:", long_options, nullptr)) !=
         -1) {
    switch (opt) {
    case OPT_DENSE:
      args.dense = true;
      break;
    case OPT_DISCRETE:
      args.discrete = true;
      break;
    case OPT_BINARY:
      args.binary = true;
      break;
    case OPT_PRINT:
      args.print = true;
      break;
    case OPT_ARENA_CAPACITY:
      if (!parse_arena_capacity(optarg, args.arena_capacity)) {
        std::cerr << "Error: arena capacity must be a positive integer within "
                     "the supported range.\n";
        return 1;
      }
      break;
    case ':':
      std::cerr << "Error: --arena-capacity requires a value.\n";
      print_usage(argv[0]);
      return 1;
    default:
      print_usage(argv[0]);
      return 1;
    }
  }

  if (optind + 2 > argc) {
    print_usage(argv[0]);
    return 1;
  }
  args.trace_file = argv[optind];
  args.properties_file = argv[optind + 1];

  if (!args.dense && !args.discrete) {
    args.dense = true; // Default to dense
  }

  if (args.binary) {
    if (args.discrete)
      discrete_binary_case(args);
    else
      dense_binary_case(args);
  } else {
    if (args.discrete)
      discrete_case(args);
    else
      dense_case(args);
  }

  return 0;
}

void dense_case(const arguments &args) {
  std::ifstream props_in(args.properties_file);
  if (!props_in.is_open()) {
    std::cerr << "Error opening properties file: " << args.properties_file
              << std::endl;
    exit(1);
  }

  std::vector<std::string> formulas;
  std::string line;
  while (std::getline(props_in, line)) {
    size_t first = line.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
      continue;
    formulas.push_back(line);
  }

  if (formulas.empty()) {
    std::cerr << "No formulas found in properties file." << std::endl;
    exit(1);
  }

  DenseMultiPropertyMonitor monitor =
      createDenseMultiPropertyMonitor(args.arena_capacity);
  ptl_parser parser;

  for (const auto &f : formulas) {
    try {
      parser.parse_dense(f, monitor);
    } catch (const std::exception &e) {
      std::cerr << "Error parsing formula: " << f << "\n"
                << e.what() << std::endl;
      exit(1);
    }
  }
  finalize_monitor(monitor);

  auto *feeder = create_dense_json_feeder(monitor, args.trace_file);
  if (!feeder) {
    std::cerr << "Error creating feeder for trace file: " << args.trace_file
              << std::endl;
    exit(1);
  }

  const std::vector<IntervalSet> *result;
  if (args.print) {
    while ((result = feed_next(feeder)) != nullptr) {
      std::cout << "{\"time\":[" << feeder_start_time(feeder) << ","
                << feeder_end_time(feeder) << "],\"verdicts\":[";
      for (size_t i = 0; i < result->size(); ++i) {
        auto intervals = toVectorIntervals((*result)[i]);
        std::cout << "[";
        for (size_t j = 0; j < intervals.size(); ++j) {
          std::cout << "[" << intervals[j].start << "," << intervals[j].end
                    << "]";
          if (j + 1 < intervals.size())
            std::cout << ",";
        }
        std::cout << "]";
        if (i + 1 < result->size())
          std::cout << ",";
      }
      std::cout << "]}\n";
    }
  } else {
    while (feed_next(feeder)) {
    }
  }

  destroy_feeder(feeder);
}

void dense_binary_case(const arguments &args) {
  std::ifstream props_in(args.properties_file);
  if (!props_in.is_open()) {
    std::cerr << "Error opening properties file: " << args.properties_file
              << std::endl;
    exit(1);
  }

  std::vector<std::string> formulas;
  std::string line;
  while (std::getline(props_in, line)) {
    size_t first = line.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
      continue;
    formulas.push_back(line);
  }

  if (formulas.empty()) {
    std::cerr << "No formulas found in properties file." << std::endl;
    exit(1);
  }

  DenseMultiPropertyMonitor monitor =
      createDenseMultiPropertyMonitor(args.arena_capacity);
  ptl_parser parser;

  for (const auto &f : formulas) {
    try {
      parser.parse_dense(f, monitor);
    } catch (const std::exception &e) {
      std::cerr << "Error parsing formula: " << f << "\n"
                << e.what() << std::endl;
      exit(1);
    }
  }
  // Finalization (with p,q,r,s ordering) is handled by the binary feeder.

  auto *feeder = create_dense_binary_feeder(monitor, args.trace_file);
  if (!feeder) {
    std::cerr << "Error creating binary feeder for trace file: "
              << args.trace_file << std::endl;
    exit(1);
  }

  const std::vector<IntervalSet> *result;
  if (args.print) {
    while ((result = feed_next(feeder)) != nullptr) {
      std::cout << "{\"time\":[" << feeder_start_time(feeder) << ","
                << feeder_end_time(feeder) << "],\"verdicts\":[";
      for (size_t i = 0; i < result->size(); ++i) {
        auto intervals = toVectorIntervals((*result)[i]);
        std::cout << "[";
        for (size_t j = 0; j < intervals.size(); ++j) {
          std::cout << "[" << intervals[j].start << "," << intervals[j].end
                    << "]";
          if (j + 1 < intervals.size())
            std::cout << ",";
        }
        std::cout << "]";
        if (i + 1 < result->size())
          std::cout << ",";
      }
      std::cout << "]}\n";
    }
  } else {
    while (feed_next(feeder)) {
    }
  }

  destroy_feeder(feeder);
}

void discrete_case(const arguments &args) {
  std::ifstream props_in(args.properties_file);
  if (!props_in.is_open()) {
    std::cerr << "Error opening properties file: " << args.properties_file
              << std::endl;
    exit(1);
  }

  std::vector<std::string> formulas;
  std::string line;
  while (std::getline(props_in, line)) {
    size_t first = line.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
      continue;
    formulas.push_back(line);
  }

  if (formulas.empty()) {
    std::cerr << "No formulas found in properties file." << std::endl;
    exit(1);
  }

  DiscreteMultiPropertyMonitor monitor =
      createDiscreteMultiPropertyMonitor(args.arena_capacity);
  ptl_parser parser;

  for (const auto &f : formulas) {
    try {
      parser.parse_discrete(f, monitor);
    } catch (const std::exception &e) {
      std::cerr << "Error parsing formula: " << f << "\n"
                << e.what() << std::endl;
      exit(1);
    }
  }
  finalize_monitor(monitor);

  auto *feeder = create_discrete_json_feeder(monitor, args.trace_file);
  if (!feeder) {
    std::cerr << "Error creating feeder for trace file: " << args.trace_file
              << std::endl;
    exit(1);
  }

  const std::vector<bool> *result;
  if (args.print) {
    while ((result = feed_next(feeder)) != nullptr) {
      std::cout << feeder_time(feeder) << ":";
      for (size_t i = 0; i < result->size(); ++i) {
        std::cout << ((*result)[i] ? "true" : "false");
        if (i + 1 < result->size())
          std::cout << ",";
      }
      std::cout << "\n";
    }
  } else {
    while (feed_next(feeder)) {
    }
  }

  destroy_feeder(feeder);
}

void discrete_binary_case(const arguments &args) {
  std::ifstream props_in(args.properties_file);
  if (!props_in.is_open()) {
    std::cerr << "Error opening properties file: " << args.properties_file
              << std::endl;
    exit(1);
  }

  std::vector<std::string> formulas;
  std::string line;
  while (std::getline(props_in, line)) {
    size_t first = line.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
      continue;
    formulas.push_back(line);
  }

  if (formulas.empty()) {
    std::cerr << "No formulas found in properties file." << std::endl;
    exit(1);
  }

  DiscreteMultiPropertyMonitor monitor =
      createDiscreteMultiPropertyMonitor(args.arena_capacity);
  ptl_parser parser;

  for (const auto &f : formulas) {
    try {
      parser.parse_discrete(f, monitor);
    } catch (const std::exception &e) {
      std::cerr << "Error parsing formula: " << f << "\n"
                << e.what() << std::endl;
      exit(1);
    }
  }
  // Finalization (with p,q,r,s ordering) is handled by the binary feeder.

  auto *feeder = create_discrete_binary_feeder(monitor, args.trace_file);
  if (!feeder) {
    std::cerr << "Error creating binary feeder for trace file: "
              << args.trace_file << std::endl;
    exit(1);
  }

  const std::vector<bool> *result;
  if (args.print) {
    while ((result = feed_next(feeder)) != nullptr) {
      std::cout << feeder_time(feeder) << ":";
      for (size_t i = 0; i < result->size(); ++i) {
        std::cout << ((*result)[i] ? "true" : "false");
        if (i + 1 < result->size())
          std::cout << ",";
      }
      std::cout << "\n";
    }
  } else {
    while (feed_next(feeder)) {
    }
  }

  destroy_feeder(feeder);
}
