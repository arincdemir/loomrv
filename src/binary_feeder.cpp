#include "loomrv/binary_feeder.hpp"
#include "loomrv/binary_row_reader.hpp"

#include <iostream>
#include <vector>

namespace loomrv {

// ---------------------------------------------------------------------------
// Dense binary feeder
// ---------------------------------------------------------------------------

struct DenseBinaryFeeder {
  DenseMultiPropertyMonitor &monitor;

  // All rows loaded upfront (reuses existing bulk reader)
  std::vector<binary_row_reader::TimescalesInput> rows;

  // Cursor: index of the *current* (new) row; previous row is cursor-1
  size_t cursor = 1;

  // Proposition values reused each tick (avoids per-row allocation)
  std::vector<bool> propInputs;

  // Last evaluated interval endpoints (for user queries)
  int lastStartTime = 0;
  int lastEndTime = 0;

  DenseBinaryFeeder(DenseMultiPropertyMonitor &mon,
                    std::vector<binary_row_reader::TimescalesInput> &&data)
      : monitor(mon), rows(std::move(data)), propInputs(4) {}
};

DenseBinaryFeeder *
create_dense_binary_feeder(DenseMultiPropertyMonitor &monitor,
                           const std::string &file_path) {
  auto rows = binary_row_reader::readInputFile(file_path);
  if (rows.empty()) {
    std::cerr << "Binary feeder: could not read or empty file: " << file_path
              << "\n";
    return nullptr;
  }

  // Auto-finalize: remap proposition leftOperandIndex to match the binary
  // struct field order: p=0, q=1, r=2, s=3.
  finalize_monitor(monitor, {"p", "q", "r", "s"});

  return new DenseBinaryFeeder(monitor, std::move(rows));
}

const std::vector<db_interval_set::IntervalSet> *
feed_next(DenseBinaryFeeder *feeder) {
  if (!feeder)
    return nullptr;

  if (feeder->cursor >= feeder->rows.size())
    return nullptr;

  const auto &prev = feeder->rows[feeder->cursor - 1];
  const auto &cur = feeder->rows[feeder->cursor];
  ++feeder->cursor;

  // Pack into vector<bool> in binary struct field order: p=0, q=1, r=2, s=3
  feeder->propInputs[0] = prev.p;
  feeder->propInputs[1] = prev.q;
  feeder->propInputs[2] = prev.r;
  feeder->propInputs[3] = prev.s;

  feeder->lastStartTime = prev.time;
  feeder->lastEndTime = cur.time;

  return &eval_multi_property(feeder->monitor, prev.time, cur.time,
                              feeder->propInputs);
}

int feeder_start_time(const DenseBinaryFeeder *feeder) {
  return feeder ? feeder->lastStartTime : 0;
}

int feeder_end_time(const DenseBinaryFeeder *feeder) {
  return feeder ? feeder->lastEndTime : 0;
}

void destroy_feeder(DenseBinaryFeeder *feeder) { delete feeder; }

// ---------------------------------------------------------------------------
// Discrete binary feeder
// ---------------------------------------------------------------------------

struct DiscreteBinaryFeeder {
  DiscreteMultiPropertyMonitor &monitor;

  std::vector<binary_row_reader::TimescalesInput> rows;
  size_t cursor = 0;

  std::vector<bool> propInputs;

  int lastTime = 0;

  DiscreteBinaryFeeder(DiscreteMultiPropertyMonitor &mon,
                       std::vector<binary_row_reader::TimescalesInput> &&data)
      : monitor(mon), rows(std::move(data)), propInputs(4) {}
};

DiscreteBinaryFeeder *
create_discrete_binary_feeder(DiscreteMultiPropertyMonitor &monitor,
                              const std::string &file_path) {
  auto rows = binary_row_reader::readInputFile(file_path);
  if (rows.empty()) {
    std::cerr << "Binary feeder: could not read or empty file: " << file_path
              << "\n";
    return nullptr;
  }

  // Auto-finalize with fixed binary field order: p=0, q=1, r=2, s=3.
  finalize_monitor(monitor, {"p", "q", "r", "s"});

  return new DiscreteBinaryFeeder(monitor, std::move(rows));
}

const std::vector<bool> *feed_next(DiscreteBinaryFeeder *feeder) {
  if (!feeder)
    return nullptr;

  if (feeder->cursor >= feeder->rows.size())
    return nullptr;

  const auto &row = feeder->rows[feeder->cursor];
  ++feeder->cursor;

  feeder->propInputs[0] = row.p;
  feeder->propInputs[1] = row.q;
  feeder->propInputs[2] = row.r;
  feeder->propInputs[3] = row.s;

  feeder->lastTime = row.time;

  return &eval_multi_property(feeder->monitor, row.time, feeder->propInputs);
}

int feeder_time(const DiscreteBinaryFeeder *feeder) {
  return feeder ? feeder->lastTime : 0;
}

void destroy_feeder(DiscreteBinaryFeeder *feeder) { delete feeder; }

} // namespace loomrv
