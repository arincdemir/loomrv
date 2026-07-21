#include "loomrv/json_feeder.hpp"
#include "simdjson.h"

#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace loomrv {

namespace {

struct PropositionUpdate {
  unsigned int nodeIndex;
  bool value;
};

const std::vector<std::pair<std::string_view, bool>> noNamedUpdates;

void apply_updates(DenseMultiPropertyMonitor &monitor,
                   const std::vector<PropositionUpdate> &updates) {
  for (const auto &update : updates) {
    monitor.nodes[update.nodeIndex].propositionValue = update.value;
  }
}

void apply_updates(DiscreteMultiPropertyMonitor &monitor,
                   const std::vector<PropositionUpdate> &updates) {
  for (const auto &update : updates) {
    monitor.nodes[update.nodeIndex].output = update.value;
  }
}

} // namespace

struct DenseJsonFeeder {
  DenseMultiPropertyMonitor &monitor;
  simdjson::padded_string json_data;
  simdjson::ondemand::parser parser;
  simdjson::ondemand::document_stream docs;
  simdjson::ondemand::document_stream::iterator it;
  bool started = false;

  // Reusable interval input; proposition updates are applied directly by index.
  TimescalesInput input;

  // Updates belonging to the row currently being parsed.
  unsigned int curTime = 0;
  std::vector<PropositionUpdate> curUpdates;

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

  // Pre-allocate the update buffer to avoid per-row allocations.
  size_t propCount = monitor.proposition_map.size();
  feeder->curUpdates.reserve(propCount);

  return feeder;
}

static bool
parse_row(simdjson::ondemand::document_reference &doc, unsigned int &time_out,
          std::vector<PropositionUpdate> &updates_out,
          const std::map<std::string, unsigned int, std::less<>>
              &proposition_map) {
  simdjson::ondemand::object obj;
  if (doc.get_object().get(obj)) {
    return false;
  }

  updates_out.clear();

  for (auto field : obj) {
    std::string_view key = field.unescaped_key();

    if (key == "time") {
      unsigned int time_val = 0;
      if (!field.value().get(time_val)) {
        time_out = time_val;
      }
    } else {
      auto proposition = proposition_map.find(key);
      if (proposition == proposition_map.end()) {
        continue;
      }
      bool val = false;
      if (!field.value().get(val)) {
        updates_out.push_back({proposition->second, val});
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

    unsigned int time_val = 0;

    if (!parse_row(doc, time_val, feeder->curUpdates,
                   feeder->monitor.proposition_map)) {
      ++(feeder->it);
      continue;
    }
    ++(feeder->it);

    if (!feeder->hasPrev) {
      feeder->curTime = time_val;
      apply_updates(feeder->monitor, feeder->curUpdates);
      feeder->hasPrev = true;
      continue;
    }

    feeder->input.startTime = feeder->curTime;
    feeder->input.endTime = time_val;

    const auto &result = eval_multi_property(feeder->monitor, feeder->input);

    feeder->lastStartTime = feeder->input.startTime;
    feeder->lastEndTime = feeder->input.endTime;

    // This row establishes the valuation for the following interval.
    feeder->curTime = time_val;
    apply_updates(feeder->monitor, feeder->curUpdates);

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
  bool initialized = false;

  // Reusable indexed update buffer.
  std::vector<PropositionUpdate> updates;

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
  feeder->updates.reserve(propCount);

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

    unsigned int time_val = 0;

    if (!parse_row(doc, time_val, feeder->updates,
                   feeder->monitor.proposition_map)) {
      ++(feeder->it);
      continue;
    }
    ++(feeder->it);

    apply_updates(feeder->monitor, feeder->updates);

    if (!feeder->initialized) {
      eval_multi_property(feeder->monitor, static_cast<int>(time_val),
                          noNamedUpdates);
      feeder->lastTime = static_cast<int>(time_val);
      feeder->initialized = true;
      continue;
    }

    const auto &result = eval_multi_property(
        feeder->monitor, static_cast<int>(time_val), noNamedUpdates);

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
