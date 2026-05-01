#pragma once

#include <string>
#include <vector>

#include "loomrv/MTLEngine.hpp"
#include "loomrv/interval_set.hpp"

namespace loomrv {

struct DenseBinaryFeeder;
struct DiscreteBinaryFeeder;

// --- Dense feeder ---

// Create a feeder that streams a binary .row.bin file.
// The monitor must already have formulas parsed but NOT yet finalized;
// the feeder will call finalize_monitor(monitor, {"p","q","r","s"})
// automatically so that proposition leftOperandIndex maps to the fixed
// binary struct field order (p=0, q=1, r=2, s=3).
// Returns nullptr on failure (file not found, etc.)
DenseBinaryFeeder *create_dense_binary_feeder(DenseMultiPropertyMonitor &monitor, const std::string &file_path);

// Advance to the next timestep.
// Returns a pointer to the monitor's internal result vector, or nullptr when
// the stream is exhausted. The pointer is valid until the next call to feed_next.
const std::vector<db_interval_set::IntervalSet> *feed_next(DenseBinaryFeeder *feeder);

// Returns the start time of the most recently evaluated interval.
int feeder_start_time(const DenseBinaryFeeder *feeder);

// Returns the end time of the most recently evaluated interval.
int feeder_end_time(const DenseBinaryFeeder *feeder);

// Free all resources associated with the feeder.
void destroy_feeder(DenseBinaryFeeder *feeder);

// --- Discrete feeder ---

// Create a feeder that streams a binary .row.bin file.
// Calls finalize_monitor(monitor, {"p","q","r","s"}) automatically.
// Returns nullptr on failure.
DiscreteBinaryFeeder *create_discrete_binary_feeder(DiscreteMultiPropertyMonitor &monitor, const std::string &file_path);

// Advance to the next timestep.
// Returns a pointer to the monitor's internal result vector, or nullptr when
// the stream is exhausted. The pointer is valid until the next call to feed_next.
const std::vector<bool> *feed_next(DiscreteBinaryFeeder *feeder);

// Returns the time of the most recently evaluated row.
int feeder_time(const DiscreteBinaryFeeder *feeder);

// Free all resources associated with the feeder.
void destroy_feeder(DiscreteBinaryFeeder *feeder);

} // namespace loomrv
