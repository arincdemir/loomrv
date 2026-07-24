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

TEST_CASE("Sparse named inputs retain proposition values and ignore unknown fields",
          "[json][named-input]") {
  SECTION("discrete time") {
    DiscreteMultiPropertyMonitor monitor =
        createDiscreteMultiPropertyMonitor(100);
    ptl_parser parser;
    parser.parse_discrete("{p}", monitor);
    parser.parse_discrete("{q}", monitor);
    finalize_monitor(monitor);

    const auto &first = eval_multi_property(
        monitor, 1,
        std::vector<std::pair<std::string_view, bool>>{
            {"p", true}, {"q", false}, {"metadata", true}});
    REQUIRE(first == std::vector<bool>{true, false});

    const auto &second = eval_multi_property(
        monitor, 2,
        std::vector<std::pair<std::string_view, bool>>{{"q", true}});
    REQUIRE(second == std::vector<bool>{true, true});

    const auto &third = eval_multi_property(
        monitor, 3,
        std::vector<std::pair<std::string_view, bool>>{
            {"p", false}, {"metadata", false}});
    REQUIRE(third == std::vector<bool>{false, true});
  }

  SECTION("dense time") {
    DenseMultiPropertyMonitor monitor = createDenseMultiPropertyMonitor(100);
    ptl_parser parser;
    parser.parse_dense("{p}", monitor);
    parser.parse_dense("{q}", monitor);
    finalize_monitor(monitor);

    const auto &first = eval_multi_property(
        monitor, TimescalesInput{0, 5,
                                 {{"p", true},
                                  {"q", false},
                                  {"metadata", true}}});
    REQUIRE(toVectorIntervals(first[0]) == std::vector<Interval>{{0, 5}});
    REQUIRE(toVectorIntervals(first[1]).empty());

    const auto &second = eval_multi_property(
        monitor, TimescalesInput{5, 10, {{"q", true}}});
    REQUIRE(toVectorIntervals(second[0]) == std::vector<Interval>{{5, 10}});
    REQUIRE(toVectorIntervals(second[1]) == std::vector<Interval>{{5, 10}});

    const auto &third = eval_multi_property(
        monitor,
        TimescalesInput{10, 15, {{"p", false}, {"metadata", false}}});
    REQUIRE(toVectorIntervals(third[0]).empty());
    REQUIRE(toVectorIntervals(third[1]) == std::vector<Interval>{{10, 15}});
  }
}

TEST_CASE("JSON feeders apply sparse updates from the first row",
          "[json][sparse-input]") {
  SECTION("discrete time initializes from the suppressed first row") {
    DiscreteMultiPropertyMonitor monitor =
        createDiscreteMultiPropertyMonitor(100);
    ptl_parser parser;
    parser.parse_discrete("{p}", monitor);
    parser.parse_discrete("{q}", monitor);
    finalize_monitor(monitor);

    DiscreteJsonFeeder *feeder = create_discrete_json_feeder(
        monitor, "data/sparse_updates.jsonl");
    REQUIRE(feeder != nullptr);

    const auto *second = feed_next(feeder);
    REQUIRE(second != nullptr);
    REQUIRE(feeder_time(feeder) == 2);
    REQUIRE(*second == std::vector<bool>{true, true});

    const auto *third = feed_next(feeder);
    REQUIRE(third != nullptr);
    REQUIRE(feeder_time(feeder) == 3);
    REQUIRE(*third == std::vector<bool>{false, true});
    REQUIRE(feed_next(feeder) == nullptr);

    destroy_feeder(feeder);
  }

  SECTION("dense time evaluates the interval established by the first row") {
    DenseMultiPropertyMonitor monitor = createDenseMultiPropertyMonitor(100);
    ptl_parser parser;
    parser.parse_dense("{p}", monitor);
    parser.parse_dense("{q}", monitor);
    finalize_monitor(monitor);

    DenseJsonFeeder *feeder = create_dense_json_feeder(
        monitor, "data/sparse_updates.jsonl");
    REQUIRE(feeder != nullptr);

    const auto *first_interval = feed_next(feeder);
    REQUIRE(first_interval != nullptr);
    REQUIRE(feeder_start_time(feeder) == 1);
    REQUIRE(feeder_end_time(feeder) == 2);
    REQUIRE(toVectorIntervals((*first_interval)[0]) ==
            std::vector<Interval>{{1, 2}});
    REQUIRE(toVectorIntervals((*first_interval)[1]).empty());

    const auto *second_interval = feed_next(feeder);
    REQUIRE(second_interval != nullptr);
    REQUIRE(feeder_start_time(feeder) == 2);
    REQUIRE(feeder_end_time(feeder) == 3);
    REQUIRE(toVectorIntervals((*second_interval)[0]) ==
            std::vector<Interval>{{2, 3}});
    REQUIRE(toVectorIntervals((*second_interval)[1]) ==
            std::vector<Interval>{{2, 3}});
    REQUIRE(feed_next(feeder) == nullptr);

    destroy_feeder(feeder);
  }
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
