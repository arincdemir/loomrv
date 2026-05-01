#pragma once

#include <vector>
#include <algorithm>
#include "loomrv/interval_set.hpp"
#include <string>
#include <limits>
#include <unordered_map>
#include <map>

#define B_INFINITY std::numeric_limits<int>::max()

namespace loomrv {

enum class NodeType {
    PROPOSITION,
    AND,
    OR,
    NOT,
    IMPLIES,
    EVENTUALLY,
    ALWAYS,
    SINCE,
    TEST,
};

struct ParsedNode {
    NodeType type;
    unsigned int leftOperandIndex;  // For propositions, this is the input index
    unsigned int rightOperandIndex; // unary operator operand sits here
    int a;
    int b;
};

struct NodeKey {
    NodeType type;
    unsigned int leftOperandIndex;
    unsigned int rightOperandIndex;
    int a;
    int b;

    bool operator==(const NodeKey &other) const {
        return type == other.type &&
               leftOperandIndex == other.leftOperandIndex &&
               rightOperandIndex == other.rightOperandIndex &&
               a == other.a &&
               b == other.b;
    }
};

struct NodeKeyHash {
    std::size_t operator()(const NodeKey &k) const {
        std::size_t h = std::hash<int>()(static_cast<int>(k.type));
        h ^= std::hash<unsigned int>()(k.leftOperandIndex) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<unsigned int>()(k.rightOperandIndex) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<int>()(k.a) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<int>()(k.b) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

struct DenseNode {
    db_interval_set::IntervalSet state;
    db_interval_set::IntervalSet output;
    NodeType type;
    unsigned int leftOperandIndex;
    unsigned int rightOperandIndex; // unary operator operand sits here
    int a;
    int b;
};

int add_with_inf(int a, int b);

db_interval_set::IntervalSet run_evaluation(std::vector<DenseNode> &nodes, db_interval_set::IntervalSetHolder &setHolder, const int startTime, const int endTime, const std::vector<bool> &propositionInputs);


struct DiscreteNode {
    db_interval_set::IntervalSet state;
    bool output;
    NodeType type;
    unsigned int leftOperandIndex;
    unsigned int rightOperandIndex; // unary operator operand sits here
    int a;
    int b;
};


bool run_evaluation(std::vector<DiscreteNode> &nodes, db_interval_set::IntervalSetHolder &setHolder, const int time, const std::vector<bool> &propositionInputs);

struct DiscreteMultiPropertyMonitor {
    bool finalized = false;
    db_interval_set::IntervalSetHolder holder{};
    std::vector<DiscreteNode> nodes;
    std::vector<ParsedNode> parsed_nodes;
    std::map<std::string, unsigned int, std::less<>> proposition_map;
    std::unordered_map<NodeKey, int, NodeKeyHash> node_dedup_map;
    unsigned int propertyCount = 0;
    std::vector<int> propertyRootNodeIndexes;
    std::vector<bool> outputs;

    DiscreteMultiPropertyMonitor() = default;

    ~DiscreteMultiPropertyMonitor() {
        if (holder.readBuffer) {
            db_interval_set::destroyHolder(holder);
            holder.readBuffer = nullptr;
            holder.writeBuffer = nullptr;
        }
    }

    // Move constructor
    DiscreteMultiPropertyMonitor(DiscreteMultiPropertyMonitor &&other) noexcept
        : finalized(other.finalized), holder(other.holder),
          nodes(std::move(other.nodes)), parsed_nodes(std::move(other.parsed_nodes)),
          proposition_map(std::move(other.proposition_map)),
          node_dedup_map(std::move(other.node_dedup_map)),
          propertyCount(other.propertyCount),
          propertyRootNodeIndexes(std::move(other.propertyRootNodeIndexes)),
          outputs(std::move(other.outputs)) {
        other.holder.readBuffer = nullptr;
        other.holder.writeBuffer = nullptr;
    }

    // Move assignment
    DiscreteMultiPropertyMonitor &operator=(DiscreteMultiPropertyMonitor &&other) noexcept {
        if (this != &other) {
            if (holder.readBuffer) {
                db_interval_set::destroyHolder(holder);
            }
            finalized = other.finalized;
            holder = other.holder;
            nodes = std::move(other.nodes);
            parsed_nodes = std::move(other.parsed_nodes);
            proposition_map = std::move(other.proposition_map);
            node_dedup_map = std::move(other.node_dedup_map);
            propertyCount = other.propertyCount;
            propertyRootNodeIndexes = std::move(other.propertyRootNodeIndexes);
            outputs = std::move(other.outputs);
            other.holder.readBuffer = nullptr;
            other.holder.writeBuffer = nullptr;
        }
        return *this;
    }

    // Disable copy (raw pointer in holder would double-free)
    DiscreteMultiPropertyMonitor(const DiscreteMultiPropertyMonitor &) = delete;
    DiscreteMultiPropertyMonitor &operator=(const DiscreteMultiPropertyMonitor &) = delete;
};

DiscreteMultiPropertyMonitor createDiscreteMultiPropertyMonitor(unsigned int holder_size);

void finalize_monitor(DiscreteMultiPropertyMonitor &monitor, std::vector<std::string> proposition_names_in_input_order);
void finalize_monitor(DiscreteMultiPropertyMonitor &monitor);

const std::vector<bool> &eval_multi_property(DiscreteMultiPropertyMonitor &monitor, const int time, const std::vector<bool> &inputs);
const std::vector<bool> &eval_multi_property(DiscreteMultiPropertyMonitor &monitor, const int time, const std::vector<std::pair<std::string_view, bool>> &propositionInputs);

bool run_evaluation(std::vector<DiscreteNode> &nodes, std::map<std::string, unsigned int, std::less<>> &proposition_map, db_interval_set::IntervalSetHolder &setHolder, const int time, const std::vector<std::pair<std::string_view, bool>> &propositionInputs);


struct DenseMultiPropertyMonitor {
    bool finalized = false;
    db_interval_set::IntervalSetHolder holder{};
    std::vector<DenseNode> nodes;
    std::vector<ParsedNode> parsed_nodes;
    std::map<std::string, unsigned int, std::less<>> proposition_map;
    std::unordered_map<NodeKey, int, NodeKeyHash> node_dedup_map;
    unsigned int propertyCount = 0;
    std::vector<int> propertyRootNodeIndexes;
    std::vector<db_interval_set::IntervalSet> outputs;

    DenseMultiPropertyMonitor() = default;

    ~DenseMultiPropertyMonitor() {
        if (holder.readBuffer) {
            db_interval_set::destroyHolder(holder);
            holder.readBuffer = nullptr;
            holder.writeBuffer = nullptr;
        }
    }

    // Move constructor
    DenseMultiPropertyMonitor(DenseMultiPropertyMonitor &&other) noexcept
        : finalized(other.finalized), holder(other.holder),
          nodes(std::move(other.nodes)), parsed_nodes(std::move(other.parsed_nodes)),
          proposition_map(std::move(other.proposition_map)),
          node_dedup_map(std::move(other.node_dedup_map)),
          propertyCount(other.propertyCount),
          propertyRootNodeIndexes(std::move(other.propertyRootNodeIndexes)),
          outputs(std::move(other.outputs)) {
        other.holder.readBuffer = nullptr;
        other.holder.writeBuffer = nullptr;
    }

    // Move assignment
    DenseMultiPropertyMonitor &operator=(DenseMultiPropertyMonitor &&other) noexcept {
        if (this != &other) {
            if (holder.readBuffer) {
                db_interval_set::destroyHolder(holder);
            }
            finalized = other.finalized;
            holder = other.holder;
            nodes = std::move(other.nodes);
            parsed_nodes = std::move(other.parsed_nodes);
            proposition_map = std::move(other.proposition_map);
            node_dedup_map = std::move(other.node_dedup_map);
            propertyCount = other.propertyCount;
            propertyRootNodeIndexes = std::move(other.propertyRootNodeIndexes);
            outputs = std::move(other.outputs);
            other.holder.readBuffer = nullptr;
            other.holder.writeBuffer = nullptr;
        }
        return *this;
    }

    // Disable copy (raw pointer in holder would double-free)
    DenseMultiPropertyMonitor(const DenseMultiPropertyMonitor &) = delete;
    DenseMultiPropertyMonitor &operator=(const DenseMultiPropertyMonitor &) = delete;
};

struct TimescalesInput {
    int startTime;
    int endTime;
    std::vector<std::pair<std::string_view, bool>> propositionInputs;
};

DenseMultiPropertyMonitor createDenseMultiPropertyMonitor(unsigned int holder_size);

void finalize_monitor(DenseMultiPropertyMonitor &monitor, std::vector<std::string> proposition_names_in_input_order);
void finalize_monitor(DenseMultiPropertyMonitor &monitor);

const std::vector<db_interval_set::IntervalSet> &eval_multi_property(DenseMultiPropertyMonitor &monitor, const int startTime, const int endTime, const std::vector<bool> &propositionInputs);

db_interval_set::IntervalSet run_evaluation(std::vector<DenseNode> &nodes, std::map<std::string, unsigned int, std::less<>> &proposition_map, db_interval_set::IntervalSetHolder &setHolder, const int startTime, const int endTime, const std::vector<std::pair<std::string_view, bool>> &propositionInputs);

const std::vector<db_interval_set::IntervalSet> &eval_multi_property(DenseMultiPropertyMonitor &monitor, const TimescalesInput &input);




} // namespace loomrv
