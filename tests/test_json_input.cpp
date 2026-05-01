#include <iostream>
#include <string_view>

#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>

#include <loomrv/MTLEngine.hpp>
#include <loomrv/binary_feeder.hpp>
#include <loomrv/interval_set.hpp>
#include <loomrv/json_feeder.hpp>
#include <loomrv/ptl.hpp>

using namespace loomrv;
using namespace db_interval_set;

TEST_CASE("AbsentAQ Json Evaluation", "[json]") {
  std::string formula =
      "historically(({r} && !{q} && once {q}) -> ( (({s} -> once[3:10] {p}) "
      "and not( not({s}) since[10:] {p})) since {q}))";

  DenseMultiPropertyMonitor monitor = createDenseMultiPropertyMonitor(500);

  ptl_parser monitorParser;
  monitorParser.parse_dense(formula, monitor);
  finalize_monitor(monitor);

  DenseJsonFeeder *feeder = create_dense_json_feeder(
      monitor, "data/fullsuite/RespondBQR/Dense1/1M/RespondBQR10.jsonl");
  REQUIRE(feeder != nullptr);

    const std::vector<IntervalSet> *result;
    while ((result = feed_next(feeder)) != nullptr) {
        REQUIRE((result->size() == 1));
        REQUIRE((toVectorIntervals((*result)[0]) == std::vector<Interval>{{feeder_start_time(feeder), feeder_end_time(feeder)}}));
    }

  destroy_feeder(feeder);
}

TEST_CASE("Discrete RespondBQR Json Feeder", "[json]") {
  std::string formula =
      "historically(({r} && !{q} && once {q}) -> ( (({s} -> once[3:10] {p}) "
      "and not( not({s}) since[10:] {p})) since {q}))";

  DiscreteMultiPropertyMonitor monitor =
      createDiscreteMultiPropertyMonitor(500);

  ptl_parser monitorParser;
  monitorParser.parse_discrete(formula, monitor);
  finalize_monitor(monitor);

  DiscreteJsonFeeder *feeder = create_discrete_json_feeder(
      monitor, "data/fullsuite/RespondBQR/Discrete/1M/RespondBQR10.jsonl");
  REQUIRE(feeder != nullptr);

  bool all_correct = true;
  const std::vector<bool> *result;
  while ((result = feed_next(feeder)) != nullptr) {
    REQUIRE((result->size() == 1));
    if (!(*result)[0]) {
      all_correct = false;
      break;
    }
  }
  REQUIRE(all_correct == true);

  destroy_feeder(feeder);
}

// ---------------------------------------------------------------------------
// Binary feeder tests
// ---------------------------------------------------------------------------

TEST_CASE("Dense RespondBQR Binary Feeder", "[binary]") {
  // RespondBQR uses p, q, r, s — exactly the binary struct field order.
  // The feeder auto-finalizes with {"p","q","r","s"}.
  std::string formula =
      "historically(({r} && !{q} && once {q}) -> ( (({s} -> once[3:10] {p}) "
      "and not( not({s}) since[10:] {p})) since {q}))";

  DenseMultiPropertyMonitor monitor = createDenseMultiPropertyMonitor(500);

  ptl_parser monitorParser;
  monitorParser.parse_dense(formula, monitor);
  // NOTE: do NOT call finalize_monitor here — the binary feeder does it.

  DenseBinaryFeeder *feeder = create_dense_binary_feeder(
      monitor, "data/fullsuite/RespondBQR/Dense10/1M/RespondBQR10.row.bin");
  REQUIRE(feeder != nullptr);

  bool all_correct = true;
  const std::vector<IntervalSet> *result;
  while ((result = feed_next(feeder)) != nullptr) {
    REQUIRE((result->size() == 1));
    if (toVectorIntervals((*result)[0]) !=
        std::vector<Interval>{
            {feeder_start_time(feeder), feeder_end_time(feeder)}}) {
      all_correct = false;
      break;
    }
  }
  REQUIRE(all_correct == true);

  destroy_feeder(feeder);
}

TEST_CASE("Discrete RespondBQR Binary Feeder", "[binary]") {
  std::string formula =
      "historically(({r} && !{q} && once {q}) -> ( (({s} -> once[3:10] {p}) "
      "and not( not({s}) since[10:] {p})) since {q}))";

  DiscreteMultiPropertyMonitor monitor =
      createDiscreteMultiPropertyMonitor(500);

  ptl_parser monitorParser;
  monitorParser.parse_discrete(formula, monitor);
  // NOTE: do NOT call finalize_monitor here — the binary feeder does it.

  DiscreteBinaryFeeder *feeder = create_discrete_binary_feeder(
      monitor, "data/fullsuite/RespondBQR/Discrete/1M/RespondBQR10.row.bin");
  REQUIRE(feeder != nullptr);

  bool all_correct = true;
  const std::vector<bool> *result;
  while ((result = feed_next(feeder)) != nullptr) {
    REQUIRE((result->size() == 1));
    if (!(*result)[0]) {
      all_correct = false;
      break;
    }
  }
  REQUIRE(all_correct == true);

  destroy_feeder(feeder);
}

TEST_CASE("Dense RecurGLB Binary Feeder", "[binary]") {
  // Single-proposition formula — only p is used; q, r, s are ignored via
  // finalize remapping.
  std::string formula = "historically(once[:10]{p})";

  DenseMultiPropertyMonitor monitor = createDenseMultiPropertyMonitor(500);

  ptl_parser monitorParser;
  monitorParser.parse_dense(formula, monitor);

  DenseBinaryFeeder *feeder = create_dense_binary_feeder(
      monitor, "data/fullsuite/RecurGLB/Dense10/1M/RecurGLB10.row.bin");
  REQUIRE(feeder != nullptr);

  bool all_correct = true;
  const std::vector<IntervalSet> *result;
  while ((result = feed_next(feeder)) != nullptr) {
    REQUIRE((result->size() == 1));
    if (toVectorIntervals((*result)[0]) !=
        std::vector<Interval>{
            {feeder_start_time(feeder), feeder_end_time(feeder)}}) {
      all_correct = false;
      break;
    }
  }
  REQUIRE(all_correct == true);

  destroy_feeder(feeder);
}