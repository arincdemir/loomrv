#pragma once

#include <string>
#include <vector>
#include <utility>

#include "loomrv/MTLEngine.hpp"
#include "loomrv/interval_set.hpp"

namespace loomrv {

struct DenseJsonFeeder;
struct DiscreteJsonFeeder;

// --- Dense feeder ---

// Create a feeder that streams NDJSON from the given file path.
// The monitor must already have formulas parsed and be finalized.
// Returns nullptr on failure (file not found, parse error, etc.)
DenseJsonFeeder *create_dense_json_feeder(DenseMultiPropertyMonitor &monitor, const std::string &file_path);

// Advance to the next timestep.
// Returns a pointer to the monitor's internal result vector, or nullptr when
// the stream is exhausted. The pointer is valid until the next call to feed_next.
const std::vector<db_interval_set::IntervalSet> *feed_next(DenseJsonFeeder *feeder);

// Returns the start time of the most recently evaluated interval.
int feeder_start_time(const DenseJsonFeeder *feeder);

// Returns the end time of the most recently evaluated interval.
int feeder_end_time(const DenseJsonFeeder *feeder);

// Free all resources associated with the feeder.
void destroy_feeder(DenseJsonFeeder *feeder);

// --- Discrete feeder ---

// Create a feeder that streams NDJSON from the given file path.
// The monitor must already have formulas parsed and be finalized.
// Returns nullptr on failure (file not found, parse error, etc.)
DiscreteJsonFeeder *create_discrete_json_feeder(DiscreteMultiPropertyMonitor &monitor, const std::string &file_path);

// Advance to the next timestep.
// Returns a pointer to the monitor's internal result vector, or nullptr when
// the stream is exhausted. The pointer is valid until the next call to feed_next.
const std::vector<bool> *feed_next(DiscreteJsonFeeder *feeder);

// Returns the time of the most recently evaluated row.
int feeder_time(const DiscreteJsonFeeder *feeder);

// Free all resources associated with the feeder.
void destroy_feeder(DiscreteJsonFeeder *feeder);

} // namespace loomrv
