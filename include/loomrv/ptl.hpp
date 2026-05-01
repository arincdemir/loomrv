/*
 * Copyright (c) 2019-2023 Dogan Ulus
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
 
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <any>

#define PEGLIB_USE_STD_ANY 1
#include <peglib.h>
#include "ptl_grammar.hpp"

#include "MTLEngine.hpp"
#include "interval_set.hpp"

namespace loomrv {



struct ptl_parser : ptl_grammar{


  peg::parser parser;
  std::vector<ParsedNode> result_nodes;
  std::map<std::string, unsigned int, std::less<>> proposition_map;
  std::unordered_map<NodeKey, int, NodeKeyHash> node_dedup_map;
  std::string last_error;

  int add_or_find_node(ParsedNode node) {
    if (node.type == NodeType::AND || node.type == NodeType::OR) {
      if (node.leftOperandIndex > node.rightOperandIndex) {
        std::swap(node.leftOperandIndex, node.rightOperandIndex);
      }
    }
    NodeKey key{node.type, node.leftOperandIndex, node.rightOperandIndex, node.a, node.b};
    auto it = node_dedup_map.find(key);
    if (it != node_dedup_map.end()) {
      return it->second;
    }
    result_nodes.push_back(node);
    int index = static_cast<int>(result_nodes.size() - 1);
    node_dedup_map[key] = index;
    return index;
  }

  explicit ptl_parser() {

    parser = peg::parser(grammar);
    parser.set_logger([this](size_t line, size_t col, const std::string &msg) {
      last_error = std::to_string(line) + ":" + std::to_string(col) + ": " + msg;
    });

    parser["NotExpr"] = [&](const peg::SemanticValues &sv) {
      int childIndex = std::any_cast<int>(sv[0]);
      ParsedNode node;
      node.type = NodeType::NOT;
      node.leftOperandIndex = 0;
      node.rightOperandIndex = childIndex;
      node.a = 0;
      node.b = 0;
      return add_or_find_node(node);
    };

    parser["Implicative"] = [&](const peg::SemanticValues &sv) {
      if (sv.size() > 1) {
        ParsedNode node;
        node.type = NodeType::IMPLIES;
        node.leftOperandIndex = std::any_cast<int>(sv[0]);
        node.rightOperandIndex = std::any_cast<int>(sv[1]);
        node.a = 0;
        node.b = 0;
        return add_or_find_node(node);
      } else {
        return std::any_cast<int>(sv[0]);
      }
    };

    parser["Disjunctive"] = [&](const peg::SemanticValues &sv) {
      if (sv.size() > 1) {
        int left = std::any_cast<int>(sv[0]);
        for (size_t i = 1; i < sv.size(); i++) {
          int right = std::any_cast<int>(sv[i]);
          ParsedNode node;
          node.type = NodeType::OR;
          node.leftOperandIndex = left;
          node.rightOperandIndex = right;
          node.a = 0;
          node.b = 0;
          left = add_or_find_node(node);
        }
        return left;
      } else {
        return std::any_cast<int>(sv[0]);
      }
    };

    parser["Conjunctive"] = [&](const peg::SemanticValues &sv) {
      if (sv.size() > 1) {
        int left = std::any_cast<int>(sv[0]);
        for (size_t i = 1; i < sv.size(); i++) {
          int right = std::any_cast<int>(sv[i]);
          ParsedNode node;
          node.type = NodeType::AND;
          node.leftOperandIndex = left;
          node.rightOperandIndex = right;
          node.a = 0;
          node.b = 0;
          left = add_or_find_node(node);
        }
        return left;
      } else {
        return std::any_cast<int>(sv[0]);
      }
    };

    parser["OnceExpr"] = [&](const peg::SemanticValues &sv) {
      int childIndex = std::any_cast<int>(sv[0]);
      ParsedNode node;
      node.type = NodeType::EVENTUALLY;
      node.leftOperandIndex = 0;
      node.rightOperandIndex = childIndex;
      node.a = 0;
      node.b = B_INFINITY;
      return add_or_find_node(node);
    };

    parser["TimedOnceExpr"] = [&](const peg::SemanticValues &sv) {
      std::pair<int, int> bound = std::any_cast<std::pair<int, int>>(sv[0]);
      int childIndex = std::any_cast<int>(sv[1]);
      ParsedNode node;
      node.type = NodeType::EVENTUALLY;
      node.leftOperandIndex = 0;
      node.rightOperandIndex = childIndex;
      node.a = bound.first;
      node.b = bound.second;
      return add_or_find_node(node);
    };

    parser["HistExpr"] = [&](const peg::SemanticValues &sv) {
      int childIndex = std::any_cast<int>(sv[0]);
      ParsedNode node;
      node.type = NodeType::ALWAYS;
      node.leftOperandIndex = 0;
      node.rightOperandIndex = childIndex;
      node.a = 0;
      node.b = B_INFINITY;
      return add_or_find_node(node);
    };

    parser["TimedHistExpr"] = [&](const peg::SemanticValues &sv) {
      std::pair<int, int> bound = std::any_cast<std::pair<int, int>>(sv[0]);
      int childIndex = std::any_cast<int>(sv[1]);
      ParsedNode node;
      node.type = NodeType::ALWAYS;
      node.leftOperandIndex = 0;
      node.rightOperandIndex = childIndex;
      node.a = bound.first;
      node.b = bound.second;
      return add_or_find_node(node);
    };

    parser["SinceExpr"] = [&](const peg::SemanticValues &sv) {
      if (sv.size() == 3) {
        int leftIndex = std::any_cast<int>(sv[0]);
        std::pair<int, int> bound = std::any_cast<std::pair<int, int>>(sv[1]);
        int rightIndex = std::any_cast<int>(sv[2]);
        ParsedNode node;
        node.type = NodeType::SINCE;
        node.leftOperandIndex = leftIndex;
        node.rightOperandIndex = rightIndex;
        node.a = bound.first;
        node.b = bound.second;
        return add_or_find_node(node);
      } else if (sv.size() == 2) {
        int leftIndex = std::any_cast<int>(sv[0]);
        int rightIndex = std::any_cast<int>(sv[1]);
        ParsedNode node;
        node.type = NodeType::SINCE;
        node.leftOperandIndex = leftIndex;
        node.rightOperandIndex = rightIndex;
        node.a = 0;
        node.b = B_INFINITY;
        return add_or_find_node(node);
      } else {
        return std::any_cast<int>(sv[0]);
      }
    };

    parser["Atom"] = [&](const peg::SemanticValues &sv) {
      std::string name = std::any_cast<std::string>(sv[0]);
      if (proposition_map.find(name) == proposition_map.end()) {
        ParsedNode node;
        node.type = NodeType::PROPOSITION;
        node.leftOperandIndex = static_cast<unsigned int>(proposition_map.size());
        node.rightOperandIndex = 0;
        node.a = 0;
        node.b = 0;
        int index = add_or_find_node(node);
        proposition_map[name] = static_cast<unsigned int>(index);
        return index;
      }
      else {
        return static_cast<int>(proposition_map[name]);
      }
      
    };

    parser["FullBound"] = [](const peg::SemanticValues &sv) {
      int l = std::stoi(std::any_cast<std::string>(sv[0]));
      int u = std::stoi(std::any_cast<std::string>(sv[1]));
      return std::make_pair(l, u);
    };

    parser["LowerBound"] = [](const peg::SemanticValues &sv) {
      int l = std::stoi(std::any_cast<std::string>(sv[0]));
      return std::make_pair(l, B_INFINITY);
    };

    parser["UpperBound"] = [](const peg::SemanticValues &sv) {
      int u = std::stoi(std::any_cast<std::string>(sv[0]));
      return std::make_pair(0, u);
    };

    parser["Name"] = [](const peg::SemanticValues &sv) { return std::string(sv.token()); };
    parser["Number"] = [](const peg::SemanticValues &sv) { return std::string(sv.token()); };

    parser.enable_packrat_parsing();
  }

  std::vector<ParsedNode> parse(const std::string &pattern) {
    result_nodes.clear();
    proposition_map.clear();
    node_dedup_map.clear();
    last_error.clear();
    bool ok = parser.parse(pattern.c_str());
    if (!ok) {
      throw std::runtime_error("Failed to parse pattern: " + pattern + (last_error.empty() ? "" : " (" + last_error + ")"));
    }
    return result_nodes;
  }

  void parse_discrete(const std::string &pattern, DiscreteMultiPropertyMonitor &monitor){
    // Load state from monitor (reuses nodes/propositions from previous parses)
    result_nodes = std::move(monitor.parsed_nodes);
    proposition_map = std::move(monitor.proposition_map);
    node_dedup_map = std::move(monitor.node_dedup_map);

    int prev_size = static_cast<int>(result_nodes.size());

    // Parse without clearing — appends to existing state
    last_error.clear();
    bool ok = parser.parse(pattern.c_str());
    if (!ok) {
      throw std::runtime_error("Failed to parse pattern: " + pattern + (last_error.empty() ? "" : " (" + last_error + ")"));
    }

    // Record root node index for this property
    monitor.propertyRootNodeIndexes.push_back(static_cast<int>(result_nodes.size() - 1));
    monitor.propertyCount++;

    // Convert only newly added ParsedNodes to DiscreteNodes
    for (size_t i = prev_size; i < result_nodes.size(); i++) {
      auto &pn = result_nodes[i];
      DiscreteNode dn;
      dn.type = pn.type;
      dn.leftOperandIndex = pn.leftOperandIndex;
      dn.rightOperandIndex = pn.rightOperandIndex;
      dn.a = pn.a;
      dn.b = pn.b;
      dn.state = db_interval_set::empty(monitor.holder);
      dn.output = false;
      monitor.nodes.push_back(dn);
    }

    // Save state back to monitor
    monitor.parsed_nodes = std::move(result_nodes);
    monitor.proposition_map = std::move(proposition_map);
    monitor.node_dedup_map = std::move(node_dedup_map);
  }

  /*
  std::vector<DenseNode> parse_dense(const std::string &pattern, db_interval_set::IntervalSetHolder &holder) {
    parse(pattern);
    std::vector<DenseNode> nodes;
    nodes.reserve(result_nodes.size());
    for (auto &pn : result_nodes) {
      DenseNode dn;
      dn.type = pn.type;
      dn.leftOperandIndex = pn.leftOperandIndex;
      dn.rightOperandIndex = pn.rightOperandIndex;
      dn.a = pn.a;
      dn.b = pn.b;
      dn.state = db_interval_set::empty(holder);
      dn.output = db_interval_set::empty(holder);
      nodes.push_back(dn);
    }
    return nodes;
  }
  */

  void parse_dense(const std::string &pattern, DenseMultiPropertyMonitor &monitor) {
    // Load state from monitor (reuses nodes/propositions from previous parses)
    result_nodes = std::move(monitor.parsed_nodes);
    proposition_map = std::move(monitor.proposition_map);
    node_dedup_map = std::move(monitor.node_dedup_map);

    int prev_size = static_cast<int>(result_nodes.size());

    // Parse without clearing — appends to existing state
    last_error.clear();
    bool ok = parser.parse(pattern.c_str());
    if (!ok) {
      throw std::runtime_error("Failed to parse pattern: " + pattern + (last_error.empty() ? "" : " (" + last_error + ")"));
    }

    // Record root node index for this property
    monitor.propertyRootNodeIndexes.push_back(static_cast<int>(result_nodes.size() - 1));
    monitor.propertyCount++;

    // Convert only newly added ParsedNodes to DenseNodes
    for (size_t i = prev_size; i < result_nodes.size(); i++) {
      auto &pn = result_nodes[i];
      DenseNode dn;
      dn.type = pn.type;
      dn.leftOperandIndex = pn.leftOperandIndex;
      dn.rightOperandIndex = pn.rightOperandIndex;
      dn.a = pn.a;
      dn.b = pn.b;
      dn.state = db_interval_set::empty(monitor.holder);
      dn.output = db_interval_set::empty(monitor.holder);
      monitor.nodes.push_back(dn);
    }

    // Save state back to monitor
    monitor.parsed_nodes = std::move(result_nodes);
    monitor.proposition_map = std::move(proposition_map);
    monitor.node_dedup_map = std::move(node_dedup_map);
  }

  const std::map<std::string, unsigned int, std::less<>>& get_proposition_map() const {
    return proposition_map;
  }

};

} // namespace loomrv
