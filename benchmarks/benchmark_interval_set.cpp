#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>

#include <limits>
#include <random>
#include <tuple>
#include <vector>

#include "loomrv/interval_set.hpp"
#include <boost/icl/interval_set.hpp>
#include <boost/icl/right_open_interval.hpp>

using namespace db_interval_set;

// --- Boost ICL Setup ---
using BoostInterval = boost::icl::right_open_interval<int>;
using BoostSet = boost::icl::interval_set<int, std::less, BoostInterval>;

static constexpr int INF = std::numeric_limits<int>::max();

// ============================================================
// Helpers
// ============================================================

static std::vector<Interval> generateRandomIntervals(std::mt19937 &gen,
                                                     int count, int maxStart,
                                                     int maxDuration) {
  std::uniform_int_distribution<> startDist(0, maxStart);
  std::uniform_int_distribution<> durationDist(1, maxDuration);

  std::vector<Interval> intervals;
  intervals.reserve(count);
  for (int i = 0; i < count; ++i) {
    int start = startDist(gen);
    int end = start + durationDist(gen);
    intervals.push_back({start, end});
  }
  return intervals;
}

static BoostSet
createBoostSetFromIntervals(const std::vector<Interval> &intervals) {
  BoostSet set;
  for (const auto &iv : intervals) {
    set.add(BoostInterval(iv.start, iv.end));
  }
  return set;
}

static std::vector<Interval> boostToVector(const BoostSet &set) {
  std::vector<Interval> result;
  for (const auto &iv : set) {
    result.push_back({iv.lower(), iv.upper()});
  }
  return result;
}

// ============================================================
// Correctness Validation
// ============================================================

TEST_CASE("Correctness: DBSet matches Boost.ICL",
          "[interval_set][correctness]") {
  std::mt19937 gen(42);
  auto intervalsA = generateRandomIntervals(gen, 500, 10000, 50);
  auto intervalsB = generateRandomIntervals(gen, 500, 10000, 50);

  BoostSet boostA = createBoostSetFromIntervals(intervalsA);
  BoostSet boostB = createBoostSetFromIntervals(intervalsB);

  auto holder = newHolder(100000);
  IntervalSet dbA = createSetFromIntervals(holder, intervalsA);
  IntervalSet dbB = createSetFromIntervals(holder, intervalsB);

  SECTION("Union produces identical results") {
    BoostSet boostResult = boostA | boostB;
    IntervalSet dbResult = unionSets(holder, dbA, dbB);
    REQUIRE(toVectorIntervals(dbResult) == boostToVector(boostResult));
  }

  SECTION("Intersection produces identical results") {
    BoostSet boostResult = boostA & boostB;
    IntervalSet dbResult = intersectSets(holder, dbA, dbB);
    REQUIRE(toVectorIntervals(dbResult) == boostToVector(boostResult));
  }

  SECTION("unionIntervalFromRight matches generic union") {
    // Build a base set with evenly-spaced intervals: [0,5), [10,15), ...,
    // [990,995)
    std::vector<Interval> baseIntervals;
    for (int i = 0; i < 100; ++i) {
      baseIntervals.push_back({i * 10, i * 10 + 5});
    }
    IntervalSet dbBase = createSetFromIntervals(holder, baseIntervals);

    // Disjoint append
    {
      INFO("Disjoint append case");
      Interval disjoint = {1005, 1010};
      IntervalSet genericResult =
          unionSets(holder, dbBase, fromInterval(holder, disjoint));
      IntervalSet optimizedResult =
          unionIntervalFromRight(holder, dbBase, disjoint);
      REQUIRE(toVectorIntervals(optimizedResult) ==
              toVectorIntervals(genericResult));
    }

    // Overlap merge
    {
      INFO("Overlap merge case");
      Interval overlap = {993, 1010};
      IntervalSet genericResult =
          unionSets(holder, dbBase, fromInterval(holder, overlap));
      IntervalSet optimizedResult =
          unionIntervalFromRight(holder, dbBase, overlap);
      REQUIRE(toVectorIntervals(optimizedResult) ==
              toVectorIntervals(genericResult));
    }

    // Empty set
    {
      INFO("Empty set case");
      IntervalSet emptySet = empty(holder);
      Interval newInterval = {100, 200};
      IntervalSet genericResult =
          unionSets(holder, emptySet, fromInterval(holder, newInterval));
      IntervalSet optimizedResult =
          unionIntervalFromRight(holder, emptySet, newInterval);
      REQUIRE(toVectorIntervals(optimizedResult) ==
              toVectorIntervals(genericResult));
    }
  }

  SECTION("checkAndClip matches includes + intersect") {
    // Build a set: [10,20), [30,40), [50,60)
    std::vector<Interval> intervals = {{10, 20}, {30, 40}, {50, 60}};
    IntervalSet dbSet = createSetFromIntervals(holder, intervals);

    // Hit case: time at first interval start
    {
      INFO("Hit case: time = 10");
      int time = 10;
      CheckAndClipResult fused = checkAndClip(holder, dbSet, time);
      bool separateOutput = includes(dbSet, time);
      IntervalSet separateClipped =
          intersectSets(holder, dbSet, fromInterval(holder, {time + 1, INF}));
      REQUIRE(fused.output == separateOutput);
      REQUIRE(toVectorIntervals(fused.set) ==
              toVectorIntervals(separateClipped));
    }

    // Miss case: time before first interval
    {
      INFO("Miss case: time = 5");
      int time = 5;
      CheckAndClipResult fused = checkAndClip(holder, dbSet, time);
      bool separateOutput = includes(dbSet, time);
      IntervalSet separateClipped =
          intersectSets(holder, dbSet, fromInterval(holder, {time + 1, INF}));
      REQUIRE(fused.output == separateOutput);
      REQUIRE(toVectorIntervals(fused.set) ==
              toVectorIntervals(separateClipped));
    }

    // Single-point interval: [10,11) — tests the skip-empty-interval logic
    {
      INFO("Single-point interval: [10,11)");
      std::vector<Interval> singlePoint = {{10, 11}, {30, 40}};
      IntervalSet singleSet = createSetFromIntervals(holder, singlePoint);
      int time = 10;
      CheckAndClipResult fused = checkAndClip(holder, singleSet, time);
      bool separateOutput = includes(singleSet, time);
      IntervalSet separateClipped = intersectSets(
          holder, singleSet, fromInterval(holder, {time + 1, INF}));
      REQUIRE(fused.output == separateOutput);
      REQUIRE(toVectorIntervals(fused.set) ==
              toVectorIntervals(separateClipped));
    }

    // Empty set
    {
      INFO("Empty set case");
      IntervalSet emptySet = empty(holder);
      int time = 42;
      CheckAndClipResult fused = checkAndClip(holder, emptySet, time);
      REQUIRE(fused.output == false);
      REQUIRE(toVectorIntervals(fused.set).empty());
    }
  }

  destroyHolder(holder);
}

// ============================================================
// Performance Benchmarks: Basic Operations (DBSet vs Boost)
// ============================================================

TEST_CASE("Basic Performance Benchmarks", "[interval_set][basic]") {
  auto N = GENERATE(50, 100, 500, 1000, 5000, 10000, 50000);

  // --- Non-overlapping intervals for Union/Intersect ---
  // Set A: even-positioned intervals [0,5), [20,25), [40,45), ...
  // Set B: odd-positioned intervals  [10,15), [30,35), [50,55), ...
  // This gives each set exactly N intervals (2*N transitions after
  // normalization), and the two sets are interleaved so the plane-sweep
  // must process all transitions from both sets.
  std::vector<Interval> nonOverlapA, nonOverlapB;
  nonOverlapA.reserve(N);
  nonOverlapB.reserve(N);
  for (int i = 0; i < N; ++i) {
    nonOverlapA.push_back({i * 20, i * 20 + 5});
    nonOverlapB.push_back({i * 20 + 10, i * 20 + 15});
  }

  // --- Random overlapping intervals for Build benchmark ---
  std::mt19937 gen(1337);
  auto randomIntervals =
      generateRandomIntervals(gen, N, N * 5, std::max(10, N / 10));

  int bufferSize = std::max(N * 16, 10000);
  auto holder = newHolder(bufferSize);

  // Pre-build DBSets for union/intersect in readBuffer
  auto dbSetA_t = createSetFromIntervals(holder, nonOverlapA);
  auto dbSetB_t = createSetFromIntervals(holder, nonOverlapB);
  swapBuffers(holder);
  IntervalSet dbSetA = {holder.readBuffer, dbSetA_t.startIndex,
                        dbSetA_t.endIndex};
  IntervalSet dbSetB = {holder.readBuffer, dbSetB_t.startIndex,
                        dbSetB_t.endIndex};

  // Pre-build Boost sets for union/intersect
  BoostSet boostSetA = createBoostSetFromIntervals(nonOverlapA);
  BoostSet boostSetB = createBoostSetFromIntervals(nonOverlapB);

  std::string sizeStr = " (N=" + std::to_string(N) + ")";

  SECTION("Build") {
    BENCHMARK("DBSet: Build" + sizeStr) {
      holder.writeIndex = 0;
      return createSetFromIntervals(holder, randomIntervals);
    };

    BENCHMARK("Boost: Build" + sizeStr) {
      return createBoostSetFromIntervals(randomIntervals);
    };
  }

  SECTION("Union") {
    BENCHMARK("DBSet: Union" + sizeStr) {
      holder.writeIndex = 0;
      return unionSets(holder, dbSetA, dbSetB);
    };

    BENCHMARK("Boost: Union" + sizeStr) { return boostSetA | boostSetB; };
  }

  SECTION("Intersection") {
    BENCHMARK("DBSet: Intersect" + sizeStr) {
      holder.writeIndex = 0;
      return intersectSets(holder, dbSetA, dbSetB);
    };

    BENCHMARK("Boost: Intersect" + sizeStr) { return boostSetA & boostSetB; };
  }

  destroyHolder(holder);
}

// ============================================================
// Serial Update Benchmark
// ============================================================

static void run_scenario_serial_update(std::vector<IntervalSet> &sets,
                                       IntervalSetHolder &holder, int steps,
                                       const std::vector<bool> &ops) {
  for (int step = 0; step < steps; step++) {
    // Copy the first set (it is not modified by any operation)
    sets[0] = copySet(holder, sets[0]);

    // Run the serial update chain
    for (size_t i = 1; i < sets.size(); i++) {
      if (ops[i]) {
        sets[i] = unionSets(holder, sets[i - 1], sets[i]);
      } else {
        sets[i] = intersectSets(holder, sets[i - 1], sets[i]);
      }
    }
    swapBuffers(holder);
  }
}

static void run_scenario_boost(std::vector<BoostSet> &sets, int steps,
                               const std::vector<bool> &ops) {
  for (int step = 0; step < steps; step++) {
    for (size_t i = 1; i < sets.size(); i++) {
      if (ops[i]) {
        sets[i] = sets[i - 1] | sets[i];
      } else {
        sets[i] = sets[i - 1] & sets[i];
      }
    }
  }
}

TEST_CASE("Serial Update Benchmark", "[interval_set][serial]") {
  auto params = GENERATE(table<int, int, int, int, int>({
      // NUM_SETS, STEPS, INTERVALS_PER_SET, MAX_START, MAX_DURATION
      {10, 100, 5, 10000, 20},   // 10 sets,  5 iv/set, sparse
      {10, 100, 5, 1000, 100},   // 10 sets,  5 iv/set, dense
      {10, 100, 50, 10000, 20},  // 10 sets, 50 iv/set, sparse
      {10, 100, 50, 1000, 100},  // 10 sets, 50 iv/set, dense
      {100, 100, 5, 10000, 20},  // 100 sets,  5 iv/set, sparse
      {100, 100, 5, 1000, 100},  // 100 sets,  5 iv/set, dense
      {100, 100, 50, 10000, 20}, // 100 sets, 50 iv/set, sparse
      {100, 100, 50, 1000, 100}, // 100 sets, 50 iv/set, dense
  }));

  int NUM_SETS, STEPS, INTERVALS_PER_SET, MAX_START, MAX_DURATION;
  std::tie(NUM_SETS, STEPS, INTERVALS_PER_SET, MAX_START, MAX_DURATION) =
      params;

  std::string benchmark_name = " (" + std::to_string(NUM_SETS) + " sets, " +
                               std::to_string(INTERVALS_PER_SET) + " iv/set, " +
                               std::to_string(STEPS) + " steps)";

  // Common data generation (deterministic)
  std::vector<bool> operations;
  std::mt19937 opGen(42);
  std::uniform_int_distribution<> opDist(0, 1);
  for (int i = 0; i < NUM_SETS; ++i) {
    operations.push_back(opDist(opGen) == 1);
  }

  std::mt19937 dataGen(1337);
  std::vector<std::vector<Interval>> allIntervals(NUM_SETS);
  for (int i = 0; i < NUM_SETS; ++i) {
    allIntervals[i] = generateRandomIntervals(dataGen, INTERVALS_PER_SET,
                                              MAX_START, MAX_DURATION);
  }

  // Buffer size: worst-case transitions for the serial chain
  const long long M = INTERVALS_PER_SET;
  const long long N = NUM_SETS;
  const long long BUFFER_SIZE_LL = M * N * (N + 1) * 2;
  const size_t BUFFER_SIZE = static_cast<size_t>(std::min(
      BUFFER_SIZE_LL, static_cast<long long>(std::numeric_limits<int>::max())));

  BENCHMARK_ADVANCED("DBSet: Serial Update" +
                     benchmark_name)(Catch::Benchmark::Chronometer meter) {
    // Allocate once per sample group (not timed)
    IntervalSetHolder holder = newHolder(BUFFER_SIZE);

    meter.measure([&] {
      // Rebuild fresh state each measurement to prevent state leakage
      holder.writeIndex = 0;
      std::vector<IntervalSet> dbSets(NUM_SETS);
      for (int i = 0; i < NUM_SETS; ++i) {
        dbSets[i] = createSetFromIntervals(holder, allIntervals[i]);
      }
      swapBuffers(holder);
      run_scenario_serial_update(dbSets, holder, STEPS, operations);
    });

    destroyHolder(holder);
  };

  BENCHMARK_ADVANCED("Boost: Serial Update" +
                     benchmark_name)(Catch::Benchmark::Chronometer meter) {
    meter.measure([&] {
      // Rebuild fresh state each measurement to prevent state leakage
      std::vector<BoostSet> boostSets(NUM_SETS);
      for (int i = 0; i < NUM_SETS; ++i) {
        boostSets[i] = createBoostSetFromIntervals(allIntervals[i]);
      }
      run_scenario_boost(boostSets, STEPS, operations);
    });
  };
}

// ============================================================
// Specialized Operation: unionIntervalFromRight vs Generic Union
// ============================================================

TEST_CASE("unionIntervalFromRight vs Generic Union",
          "[interval_set][specialized]") {
  auto count = GENERATE(50, 100, 500, 1000, 5000, 10000, 50000);

  IntervalSetHolder holder = newHolder(count * 12);

  // Create evenly-spaced base set: [0,5), [10,15), ..., [(N-1)*10, (N-1)*10+5)
  std::vector<Interval> initialIntervals;
  initialIntervals.reserve(count);
  for (int i = 0; i < count; ++i) {
    initialIntervals.push_back({i * 10, i * 10 + 5});
  }

  IntervalSet heavySet_t = createSetFromIntervals(holder, initialIntervals);
  swapBuffers(holder);
  IntervalSet heavySet = {holder.readBuffer, heavySet_t.startIndex,
                          heavySet_t.endIndex};
  int baseWriteIndex = holder.writeIndex;

  std::string sizeStr = " (N=" + std::to_string(count) + ")";

  // --- Disjoint: new interval after all existing ---
  int lastEnd = (count - 1) * 10 + 5;
  Interval disjointInterval = {lastEnd + 10, lastEnd + 20};

  BENCHMARK("Disjoint: Generic unionSets" + sizeStr) {
    holder.writeIndex = baseWriteIndex;
    IntervalSet temp = fromInterval(holder, disjointInterval);
    return unionSets(holder, heavySet, temp);
  };

  BENCHMARK("Disjoint: unionIntervalFromRight" + sizeStr) {
    holder.writeIndex = baseWriteIndex;
    return unionIntervalFromRight(holder, heavySet, disjointInterval);
  };

  // --- Overlap: new interval overlapping the last ---
  Interval overlapInterval = {lastEnd - 2, lastEnd + 20};

  BENCHMARK("Overlap: Generic unionSets" + sizeStr) {
    holder.writeIndex = baseWriteIndex;
    IntervalSet temp = fromInterval(holder, overlapInterval);
    return unionSets(holder, heavySet, temp);
  };

  BENCHMARK("Overlap: unionIntervalFromRight" + sizeStr) {
    holder.writeIndex = baseWriteIndex;
    return unionIntervalFromRight(holder, heavySet, overlapInterval);
  };

  // --- Empty set: common case when temporal node hasn't triggered yet ---
  BENCHMARK("Empty: Generic unionSets" + sizeStr) {
    holder.writeIndex = baseWriteIndex;
    IntervalSet emptySet = empty(holder);
    IntervalSet temp = fromInterval(holder, disjointInterval);
    return unionSets(holder, emptySet, temp);
  };

  BENCHMARK("Empty: unionIntervalFromRight" + sizeStr) {
    holder.writeIndex = baseWriteIndex;
    IntervalSet emptySet = empty(holder);
    return unionIntervalFromRight(holder, emptySet, disjointInterval);
  };

  destroyHolder(holder);
}

// ============================================================
// Specialized Operation: checkAndClip vs Separate Operations
// ============================================================

TEST_CASE("checkAndClip vs Separate Operations",
          "[interval_set][specialized]") {
  auto count = GENERATE(10, 50, 100, 500, 1000, 5000);

  IntervalSetHolder holder = newHolder(count * 12);

  // Simulate temporal operator state: intervals starting at time 100
  // [100,110), [120,130), ..., [100+(N-1)*20, 110+(N-1)*20)
  int baseTime = 100;
  std::vector<Interval> intervals;
  intervals.reserve(count);
  for (int i = 0; i < count; ++i) {
    intervals.push_back({baseTime + i * 20, baseTime + i * 20 + 10});
  }

  IntervalSet stateSet_t = createSetFromIntervals(holder, intervals);
  swapBuffers(holder);
  IntervalSet stateSet = {holder.readBuffer, stateSet_t.startIndex,
                          stateSet_t.endIndex};
  int baseWriteIndex = holder.writeIndex;

  std::string sizeStr = " (N=" + std::to_string(count) + ")";

  // --- Hit case: time == first interval start (included in the set) ---
  int hitTime = baseTime;

  BENCHMARK("Hit: includes + intersect" + sizeStr) {
    holder.writeIndex = baseWriteIndex;
    bool output = includes(stateSet, hitTime);
    IntervalSet clipped = intersectSets(
        holder, stateSet, fromInterval(holder, {hitTime + 1, INF}));
    return std::make_pair(output, clipped.endIndex);
  };

  BENCHMARK("Hit: checkAndClip" + sizeStr) {
    holder.writeIndex = baseWriteIndex;
    CheckAndClipResult result = checkAndClip(holder, stateSet, hitTime);
    return std::make_pair(result.output, result.set.endIndex);
  };

  // --- Miss case: time before first interval (not included) ---
  int missTime = baseTime - 5;

  BENCHMARK("Miss: includes + intersect" + sizeStr) {
    holder.writeIndex = baseWriteIndex;
    bool output = includes(stateSet, missTime);
    IntervalSet clipped = intersectSets(
        holder, stateSet, fromInterval(holder, {missTime + 1, INF}));
    return std::make_pair(output, clipped.endIndex);
  };

  BENCHMARK("Miss: checkAndClip" + sizeStr) {
    holder.writeIndex = baseWriteIndex;
    CheckAndClipResult result = checkAndClip(holder, stateSet, missTime);
    return std::make_pair(result.output, result.set.endIndex);
  };

  destroyHolder(holder);
}