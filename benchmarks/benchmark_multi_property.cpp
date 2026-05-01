#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include "loomrv/MTLEngine.hpp"
#include "loomrv/interval_set.hpp"
#include "loomrv/json_feeder.hpp"
#include "loomrv/ptl.hpp"

namespace fs = std::filesystem;
using namespace loomrv;
using namespace db_interval_set;

// Extract the pattern value from a simple YAML file.
// Expects a line like:  pattern : "..."
static std::string extract_pattern(const std::string &yaml_path) {
  std::ifstream in(yaml_path);
  if (!in.is_open()) {
    throw std::runtime_error("Cannot open YAML file: " + yaml_path);
  }
  std::string line;
  // Match: pattern : "..." (possibly with leading spaces, colon spacing)
  std::regex pat_re(R"(^\s*pattern\s*:\s*\"(.*)\"\s*$)");
  while (std::getline(in, line)) {
    std::smatch m;
    if (std::regex_match(line, m, pat_re)) {
      return m[1].str();
    }
  }
  throw std::runtime_error("No pattern found in YAML file: " + yaml_path);
}

// Collect all formulas from data/fullsuite/*/Dense1/1M/*.yaml
static std::vector<std::string> collect_all_formulas() {
  std::vector<std::string> formulas;
  std::string base = "data/fullsuite";

  // Gather YAML files sorted for deterministic ordering
  std::vector<std::string> yaml_paths;
  for (auto &entry : fs::recursive_directory_iterator(base)) {
    if (!entry.is_regular_file())
      continue;
    auto p = entry.path();
    if (p.extension() != ".yaml")
      continue;
    // Only Dense1/1M yaml files
    std::string ps = p.string();
    if (ps.find("/Dense1/1M/") == std::string::npos)
      continue;
    yaml_paths.push_back(ps);
  }
  std::sort(yaml_paths.begin(), yaml_paths.end());

  for (auto &yp : yaml_paths) {
    formulas.push_back(extract_pattern(yp));
  }
  return formulas;
}

static const std::string INPUT_FILE =
    "data/fullsuite/RespondBQR/Dense1/1M/RespondBQR10.jsonl";

TEST_CASE("Multi-property anti-redundancy benchmark", "[multi_property]") {

  auto formulas = collect_all_formulas();
  REQUIRE(formulas.size() == 30);

  std::cout << "\n=== Collected " << formulas.size() << " formulas ===\n";

  // --- Benchmark 1: 30 separate monitors, processed sequentially ---
  SECTION("30 separate monitors") {
    // Build monitors once just to count nodes
    {
      std::vector<DenseMultiPropertyMonitor> monitors;
      monitors.reserve(formulas.size());
      ptl_parser parser;
      size_t total_nodes = 0;
      for (auto &f : formulas) {
        monitors.push_back(createDenseMultiPropertyMonitor(10000));
        auto &mon = monitors.back();
        parser.parse_dense(f, mon);
        finalize_monitor(mon);
        total_nodes += mon.nodes.size();
      }
      std::cout << "[Separate] Total nodes across 30 monitors: " << total_nodes
                << "\n";
    }

    BENCHMARK_ADVANCED("30 separate monitors")(
        Catch::Benchmark::Chronometer meter) {
      meter.measure([&] {
        // Build fresh monitors each iteration (temporal state is not reusable)
        std::vector<DenseMultiPropertyMonitor> bench_monitors;
        bench_monitors.reserve(formulas.size());
        ptl_parser bench_parser;
        for (auto &f : formulas) {
          bench_monitors.push_back(createDenseMultiPropertyMonitor(10000));
          auto &mon = bench_monitors.back();
          bench_parser.parse_dense(f, mon);
          finalize_monitor(mon);
        }

        std::vector<DenseJsonFeeder *> feeders;
        feeders.reserve(bench_monitors.size());
        for (auto &mon : bench_monitors) {
          auto *feeder = create_dense_json_feeder(mon, INPUT_FILE);
          feeders.push_back(feeder);
        }

        bool any_active = true;
        while (any_active) {
          any_active = false;
          for (size_t i = 0; i < feeders.size(); i++) {
            if (feeders[i] && feed_next(feeders[i]) != nullptr) {
              any_active = true;
            }
          }
        }

        for (auto *f : feeders) {
          destroy_feeder(f);
        }
        return any_active;
      });
    };
  }

  // --- Benchmark 2: One big monitor with all 30 formulas ---
  SECTION("1 combined monitor") {
    // Build monitor once just to count nodes
    {
      DenseMultiPropertyMonitor count_monitor =
          createDenseMultiPropertyMonitor(10000);
      ptl_parser parser;
      for (auto &f : formulas) {
        parser.parse_dense(f, count_monitor);
      }
      finalize_monitor(count_monitor);
      std::cout << "[Combined] Total nodes in single monitor: "
                << count_monitor.nodes.size() << "\n";
    }

    BENCHMARK_ADVANCED("1 combined monitor")(
        Catch::Benchmark::Chronometer meter) {
      meter.measure([&] {
        // Build fresh monitor each iteration (temporal state is not reusable)
        DenseMultiPropertyMonitor bench_monitor =
            createDenseMultiPropertyMonitor(10000);
        ptl_parser bench_parser;
        for (auto &f : formulas) {
          bench_parser.parse_dense(f, bench_monitor);
        }
        finalize_monitor(bench_monitor);

        auto *feeder = create_dense_json_feeder(bench_monitor, INPUT_FILE);
        while (feed_next(feeder)) {
          // consume
        }
        destroy_feeder(feeder);
        return 0;
      });
    };
  }
}
