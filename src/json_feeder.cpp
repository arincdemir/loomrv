#include "loomrv/json_feeder.hpp"
#include "simdjson.h"

#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace loomrv {

struct DenseJsonFeeder {
  DenseMultiPropertyMonitor &monitor;
  simdjson::padded_string json_data;
  simdjson::ondemand::parser parser;
  simdjson::ondemand::document_stream docs;
  simdjson::ondemand::document_stream::iterator it;
  bool started = false;

  // Reusable input struct (avoids allocation per iteration)
  TimescalesInput input;

  // Current timestep state
  unsigned int curTime = 0;
  std::vector<std::pair<std::string_view, bool>> curPropositions;

  // Whether we have consumed at least one row (need two rows to produce output)
  bool hasPrev = false;

  // Last evaluated interval (for user queries)
  int lastStartTime = 0;
  int lastEndTime = 0;

  DenseJsonFeeder(DenseMultiPropertyMonitor &mon,
                  simdjson::padded_string &&data)
      : monitor(mon), json_data(std::move(data)) {}
};

DenseJsonFeeder *create_dense_json_feeder(DenseMultiPropertyMonitor &monitor,
                                          const std::string &file_path) {
  simdjson::padded_string data;
  if (simdjson::padded_string::load(file_path).get(data)) {
    std::cerr << "Could not load file: " << file_path << "\n";
    return nullptr;
  }

  auto *feeder = new DenseJsonFeeder(monitor, std::move(data));

  if (feeder->parser.iterate_many(feeder->json_data).get(feeder->docs)) {
    std::cerr << "Failed to parse NDJSON: " << file_path << "\n";
    delete feeder;
    return nullptr;
  }

  feeder->it = feeder->docs.begin();
  feeder->started = true;

  // Pre-allocate proposition vectors to avoid per-row allocations
  size_t propCount = monitor.proposition_map.size();
  feeder->curPropositions.reserve(propCount);
  feeder->input.propositionInputs.reserve(propCount);

  return feeder;
}

static bool
parse_row(simdjson::ondemand::document_reference &doc, unsigned int &time_out,
          std::vector<std::pair<std::string_view, bool>> &props_out) {
  simdjson::ondemand::object obj;
  if (doc.get_object().get(obj)) {
    return false;
  }

  props_out.clear();

  for (auto field : obj) {
    std::string_view key = field.unescaped_key();

    if (key == "time") {
      unsigned int time_val = 0;
      if (!field.value().get(time_val)) {
        time_out = time_val;
      }
    } else {
      bool val = false;
      if (!field.value().get(val)) {
        props_out.push_back({key, val});
      }
    }
  }
  return true;
}

const std::vector<db_interval_set::IntervalSet> *feed_next(DenseJsonFeeder *feeder) {
  if (!feeder || !feeder->started)
    return nullptr;

  while (feeder->it != feeder->docs.end()) {
    simdjson::ondemand::document_reference doc = *feeder->it;
    ++(feeder->it);

    unsigned int time_val = 0;

    if (!parse_row(doc, time_val, feeder->curPropositions)) {
      continue;
    }

    if (!feeder->hasPrev) {
      feeder->curTime = time_val;
      // Move cur into input for safe-keeping, cur will be repopulated next
      // iteration
      feeder->input.propositionInputs.swap(feeder->curPropositions);
      feeder->hasPrev = true;
      continue;
    }

    // Previous is already in input.propositionInputs from last iteration
    feeder->input.startTime = feeder->curTime;
    feeder->input.endTime = time_val;

    const auto &result = eval_multi_property(feeder->monitor, feeder->input);

    feeder->lastStartTime = feeder->input.startTime;
    feeder->lastEndTime = feeder->input.endTime;

    // Current becomes next previous: swap into input
    feeder->curTime = time_val;
    feeder->input.propositionInputs.swap(feeder->curPropositions);

    return &result;
  }

  return nullptr;
}

int feeder_start_time(const DenseJsonFeeder *feeder) {
  return feeder ? feeder->lastStartTime : 0;
}

int feeder_end_time(const DenseJsonFeeder *feeder) {
  return feeder ? feeder->lastEndTime : 0;
}

void destroy_feeder(DenseJsonFeeder *feeder) { delete feeder; }

// --- Discrete feeder ---

struct DiscreteJsonFeeder {
  DiscreteMultiPropertyMonitor &monitor;
  simdjson::padded_string json_data;
  simdjson::ondemand::parser parser;
  simdjson::ondemand::document_stream docs;
  simdjson::ondemand::document_stream::iterator it;
  bool started = false;

  // Reusable buffer
  std::vector<std::pair<std::string_view, bool>> namedProps;

  int lastTime = 0;

  DiscreteJsonFeeder(DiscreteMultiPropertyMonitor &mon,
                     simdjson::padded_string &&data)
      : monitor(mon), json_data(std::move(data)) {}
};

DiscreteJsonFeeder *
create_discrete_json_feeder(DiscreteMultiPropertyMonitor &monitor,
                            const std::string &file_path) {
  simdjson::padded_string data;
  if (simdjson::padded_string::load(file_path).get(data)) {
    std::cerr << "Could not load file: " << file_path << "\n";
    return nullptr;
  }

  auto *feeder = new DiscreteJsonFeeder(monitor, std::move(data));

  if (feeder->parser.iterate_many(feeder->json_data).get(feeder->docs)) {
    std::cerr << "Failed to parse NDJSON: " << file_path << "\n";
    delete feeder;
    return nullptr;
  }

  feeder->it = feeder->docs.begin();
  feeder->started = true;

  size_t propCount = monitor.proposition_map.size();
  feeder->namedProps.reserve(propCount);

  return feeder;
}

// TODO we changed this to return a vector referance instead of filling the
// output vector. the vector stays usable till the next feed call. the caller
// can decide what to do with it. did this for optimization purposes
const std::vector<bool> *feed_next(DiscreteJsonFeeder *feeder) {
  if (!feeder || !feeder->started)
    return nullptr;

  while (feeder->it != feeder->docs.end()) {
    simdjson::ondemand::document_reference doc = *feeder->it;
    ++(feeder->it);

    unsigned int time_val = 0;

    if (!parse_row(doc, time_val, feeder->namedProps)) {
      continue;
    }

    const auto &result = eval_multi_property(
        feeder->monitor, static_cast<int>(time_val), feeder->namedProps);

    feeder->lastTime = static_cast<int>(time_val);

    return &result;
  }

  return nullptr;
}

int feeder_time(const DiscreteJsonFeeder *feeder) {
  return feeder ? feeder->lastTime : 0;
}

void destroy_feeder(DiscreteJsonFeeder *feeder) { delete feeder; }

} // namespace loomrv
