#include "loomrv/MTLEngine.hpp"

namespace loomrv {

int add_with_inf(int a, int b) {
    if (a == B_INFINITY || b == B_INFINITY) {
        return B_INFINITY;
    }
    else {
        return a + b;
    }
}

db_interval_set::IntervalSet run_evaluation(std::vector<DenseNode> &nodes, db_interval_set::IntervalSetHolder &setHolder, const int startTime, const int endTime, const std::vector<bool> &propositionInputs) {
    int nodeCount = nodes.size();
    for(size_t node_index = 0; node_index < nodeCount; node_index++) {
        DenseNode &curNode = nodes[node_index];
        switch (curNode.type)
        {          
        case NodeType::PROPOSITION:
            if (propositionInputs[curNode.leftOperandIndex]) curNode.output = db_interval_set::fromInterval(setHolder, {startTime, endTime});
            else curNode.output = db_interval_set::empty(setHolder);
            break;
        case NodeType::AND:
            curNode.output = db_interval_set::intersectSets(setHolder, nodes[curNode.leftOperandIndex].output, nodes[curNode.rightOperandIndex].output);
            break;
        case NodeType::OR:
            curNode.output = db_interval_set::unionSets(setHolder, nodes[curNode.leftOperandIndex].output, nodes[curNode.rightOperandIndex].output);
            break;
        case NodeType::NOT:
            curNode.output = db_interval_set::negateSet(setHolder, nodes[curNode.rightOperandIndex].output, {startTime, endTime});
            break;    
        case NodeType::IMPLIES:
        {
            // A IMPLIES B is equivalent to (NOT A) OR B
            auto notLeft = db_interval_set::negateSet(setHolder, nodes[curNode.leftOperandIndex].output,
                                                                {startTime, endTime});
            
            auto right = nodes[curNode.rightOperandIndex].output;
            curNode.output = db_interval_set::unionSets(setHolder, notLeft, right);
            break;
        } 
        
        case NodeType::EVENTUALLY:
            {
            auto rightOutput = nodes[curNode.rightOperandIndex].output;

            curNode.output = db_interval_set::empty(setHolder);

            auto iterator = db_interval_set::createSegmentIterator(db_interval_set::empty(setHolder), rightOutput, {startTime, endTime});
            int i = 0;
            while (db_interval_set::getNextSegment(iterator)) {
                if (iterator.interval.end == iterator.interval.start) continue;
                if (iterator.rightTruthy) {
                    curNode.state = db_interval_set::unionSets(setHolder, curNode.state,
                        db_interval_set::fromInterval(setHolder, {iterator.interval.start + curNode.a, add_with_inf(iterator.interval.end, curNode.b)}));
                }
                i++;
                

                auto segmentOutput = db_interval_set::intersectSets(setHolder, curNode.state,
                    db_interval_set::fromInterval(setHolder, {iterator.interval.start, iterator.interval.end}));
                curNode.output = db_interval_set::unionSets(setHolder, curNode.output, segmentOutput);
                
            }
            curNode.state = db_interval_set::intersectSets(setHolder, curNode.state,
                db_interval_set::fromInterval(setHolder, {endTime, B_INFINITY}));
            break;
        }
        case NodeType::ALWAYS:
            {
            auto rightOutput = nodes[curNode.rightOperandIndex].output;

            curNode.output = db_interval_set::empty(setHolder);

            auto iterator = db_interval_set::createSegmentIterator(db_interval_set::empty(setHolder), rightOutput, {startTime, endTime});
            
            while (db_interval_set::getNextSegment(iterator)) {
                if (iterator.interval.end == iterator.interval.start) continue;
                if (!iterator.rightTruthy) {
                    curNode.state = db_interval_set::unionSets(setHolder, curNode.state,
                        db_interval_set::fromInterval(setHolder, {iterator.interval.start + curNode.a, add_with_inf(iterator.interval.end, curNode.b)}));
                }

                auto segmentOutput = db_interval_set::negateSet(setHolder, curNode.state, {iterator.interval.start, iterator.interval.end});
                curNode.output = db_interval_set::unionSets(setHolder, curNode.output, segmentOutput);
                
            }
            curNode.state = db_interval_set::intersectSets(setHolder, curNode.state,
                db_interval_set::fromInterval(setHolder, {endTime, B_INFINITY}));
            break;
        }
        case NodeType::SINCE:
            {
            auto leftOutput = nodes[curNode.leftOperandIndex].output;
            auto rightOutput = nodes[curNode.rightOperandIndex].output;
            curNode.output = db_interval_set::empty(setHolder);

            auto iterator = db_interval_set::createSegmentIterator(leftOutput, rightOutput, {startTime, endTime});
            
            while (db_interval_set::getNextSegment(iterator)) {
                if (iterator.interval.end == iterator.interval.start) continue;
                if (iterator.leftTruthy && iterator.rightTruthy) {
                    curNode.state = db_interval_set::unionSets(setHolder, curNode.state,
                        db_interval_set::fromInterval(setHolder, {iterator.interval.start + curNode.a, add_with_inf(iterator.interval.end, curNode.b)}));
                }
                else if (!iterator.leftTruthy && iterator.rightTruthy) {
                    curNode.state = db_interval_set::fromInterval(setHolder, {iterator.interval.end + curNode.a, add_with_inf(iterator.interval.end, curNode.b)});
                }
                else if (iterator.leftTruthy && !iterator.rightTruthy) {
                }
                else {
                    curNode.state = db_interval_set::empty(setHolder);
                }

                auto segmentOutput = db_interval_set::intersectSets(setHolder, curNode.state,
                    db_interval_set::fromInterval(setHolder, {iterator.interval.start, iterator.interval.end}));
                curNode.output = db_interval_set::unionSets(setHolder, curNode.output, segmentOutput);
                
            }
            curNode.state = db_interval_set::intersectSets(setHolder, curNode.state,
                db_interval_set::fromInterval(setHolder, {endTime, B_INFINITY}));
            break;
            }
        case NodeType::TEST:
            break;
        }
    
    }
    return nodes[nodeCount - 1].output;
}


bool run_evaluation(std::vector<DiscreteNode> &nodes, db_interval_set::IntervalSetHolder &setHolder, const int time, const std::vector<bool> &propositionInputs) {
    int nodeCount = nodes.size();
    for (unsigned int node_index = 0; node_index < nodeCount; node_index++) {
        DiscreteNode &curNode = nodes[node_index];
        switch (curNode.type)
        {
        case NodeType::PROPOSITION:
            curNode.output = propositionInputs[curNode.leftOperandIndex];
            break;
        case NodeType::AND:
            curNode.output = nodes[curNode.leftOperandIndex].output && nodes[curNode.rightOperandIndex].output;
            break;
        case NodeType::OR:
            curNode.output = nodes[curNode.leftOperandIndex].output || nodes[curNode.rightOperandIndex].output;
            break;
        case NodeType::NOT:
            curNode.output = !nodes[curNode.rightOperandIndex].output;
            break;    
        case NodeType::IMPLIES:
            curNode.output = !(nodes[curNode.leftOperandIndex].output && !nodes[curNode.rightOperandIndex].output);
            break; 
        case NodeType::EVENTUALLY:
        {
            if (nodes[curNode.rightOperandIndex].output) {
                curNode.state = db_interval_set::unionIntervalFromRight(setHolder, curNode.state, {time + curNode.a, add_with_inf(time + 1, curNode.b)});
            }
            db_interval_set::CheckAndClipResult result = db_interval_set::checkAndClip(setHolder, curNode.state, time);
            curNode.output = result.output;
            curNode.state = result.set;
            break;

        }   
        case NodeType::ALWAYS:
        {
            if (!nodes[curNode.rightOperandIndex].output) {
                curNode.state = db_interval_set::unionIntervalFromRight(setHolder, curNode.state, {time + curNode.a, add_with_inf(time + 1, curNode.b)});
            }
            db_interval_set::CheckAndClipResult result = db_interval_set::checkAndClip(setHolder, curNode.state, time);
            curNode.output = !result.output;
            curNode.state = result.set;
            // curNode.output = !db_interval_set::includes(curNode.state, time);
            // curNode.state = db_interval_set::intersectSets(setHolder, curNode.state,
            //     db_interval_set::fromInterval(setHolder, {time + 1, B_INFINITY}));
            break;
        }
        case NodeType::SINCE:
        {
            bool leftOutput = nodes[curNode.leftOperandIndex].output;
            bool rightOutput = nodes[curNode.rightOperandIndex].output;
            if (leftOutput && rightOutput) {
                curNode.state = db_interval_set::unionIntervalFromRight(setHolder, curNode.state, {time + curNode.a, add_with_inf(time + 1, curNode.b)});
            }
            else if (!leftOutput && rightOutput) {
                curNode.state = db_interval_set::fromInterval(setHolder, {time + curNode.a, add_with_inf(time + 1, curNode.b)});
            }
            else if (leftOutput && !rightOutput) {

            }
            else {
                curNode.state = db_interval_set::empty(setHolder);
            }
            db_interval_set::CheckAndClipResult result = db_interval_set::checkAndClip(setHolder, curNode.state, time);
            curNode.output = result.output;
            curNode.state = result.set;
            // curNode.output = db_interval_set::includes(curNode.state, time);
            // curNode.state = db_interval_set::intersectSets(setHolder, curNode.state,
            //     db_interval_set::fromInterval(setHolder, {time + 1, B_INFINITY}));
            break;
        }
        case NodeType::TEST:
            break;
        }
    }
    return nodes[nodeCount - 1].output;
    
}



bool run_evaluation(std::vector<DiscreteNode> &nodes, std::map<std::string, unsigned int, std::less<>> &proposition_map, db_interval_set::IntervalSetHolder &setHolder, const int time, const std::vector<std::pair<std::string_view, bool>> &propositionInputs) {
    for (const auto &propInput : propositionInputs) {
        auto &propNode = nodes[proposition_map.find(propInput.first)->second];
        propNode.output = propInput.second;
    }

    int nodeCount = nodes.size();
    for (unsigned int node_index = 0; node_index < nodeCount; node_index++) {
        DiscreteNode &curNode = nodes[node_index];
        switch (curNode.type)
        {
        case NodeType::PROPOSITION:
            break;
        case NodeType::AND:
            curNode.output = nodes[curNode.leftOperandIndex].output && nodes[curNode.rightOperandIndex].output;
            break;
        case NodeType::OR:
            curNode.output = nodes[curNode.leftOperandIndex].output || nodes[curNode.rightOperandIndex].output;
            break;
        case NodeType::NOT:
            curNode.output = !nodes[curNode.rightOperandIndex].output;
            break;
        case NodeType::IMPLIES:
            curNode.output = !(nodes[curNode.leftOperandIndex].output && !nodes[curNode.rightOperandIndex].output);
            break;
        case NodeType::EVENTUALLY:
        {
            if (nodes[curNode.rightOperandIndex].output) {
                curNode.state = db_interval_set::unionIntervalFromRight(setHolder, curNode.state, {time + curNode.a, add_with_inf(time + 1, curNode.b)});
            }
            db_interval_set::CheckAndClipResult result = db_interval_set::checkAndClip(setHolder, curNode.state, time);
            curNode.output = result.output;
            curNode.state = result.set;
            break;
        }
        case NodeType::ALWAYS:
        {
            if (!nodes[curNode.rightOperandIndex].output) {
                curNode.state = db_interval_set::unionIntervalFromRight(setHolder, curNode.state, {time + curNode.a, add_with_inf(time + 1, curNode.b)});
            }
            db_interval_set::CheckAndClipResult result = db_interval_set::checkAndClip(setHolder, curNode.state, time);
            curNode.output = !result.output;
            curNode.state = result.set;
            break;
        }
        case NodeType::SINCE:
        {
            bool leftOutput = nodes[curNode.leftOperandIndex].output;
            bool rightOutput = nodes[curNode.rightOperandIndex].output;
            if (leftOutput && rightOutput) {
                curNode.state = db_interval_set::unionIntervalFromRight(setHolder, curNode.state, {time + curNode.a, add_with_inf(time + 1, curNode.b)});
            }
            else if (!leftOutput && rightOutput) {
                curNode.state = db_interval_set::fromInterval(setHolder, {time + curNode.a, add_with_inf(time + 1, curNode.b)});
            }
            else if (leftOutput && !rightOutput) {
            }
            else {
                curNode.state = db_interval_set::empty(setHolder);
            }
            db_interval_set::CheckAndClipResult result = db_interval_set::checkAndClip(setHolder, curNode.state, time);
            curNode.output = result.output;
            curNode.state = result.set;
            break;
        }
        case NodeType::TEST:
            break;
        }
    }
    return nodes[nodeCount - 1].output;
}

DiscreteMultiPropertyMonitor createDiscreteMultiPropertyMonitor(unsigned int holder_size) {
    DiscreteMultiPropertyMonitor monitor;
    monitor.holder = db_interval_set::newHolder(holder_size);    
    return monitor;
}

void finalize_monitor(DiscreteMultiPropertyMonitor &monitor, std::vector<std::string> proposition_names_in_input_order) {
    // Remap proposition leftOperandIndex to match the user's input order
    for (size_t i = 0; i < proposition_names_in_input_order.size(); i++) {
        auto it = monitor.proposition_map.find(proposition_names_in_input_order[i]);
        if (it != monitor.proposition_map.end()) {
            monitor.nodes[it->second].leftOperandIndex = static_cast<unsigned int>(i);
        }
    }
    monitor.outputs.reserve(monitor.propertyCount);
    monitor.finalized = true;
}

void finalize_monitor(DiscreteMultiPropertyMonitor &monitor) {
    monitor.outputs.reserve(monitor.propertyCount);
    monitor.finalized = true;
}

const std::vector<bool> &eval_multi_property(DiscreteMultiPropertyMonitor &monitor, const int time, const std::vector<bool> &inputs) {
    db_interval_set::swapBuffers(monitor.holder);
    run_evaluation(monitor.nodes, monitor.holder, time, inputs);
    monitor.outputs.clear();
    for (int rootIdx : monitor.propertyRootNodeIndexes) {
        monitor.outputs.push_back(monitor.nodes[rootIdx].output);
    }
    return monitor.outputs;
}

const std::vector<bool> &eval_multi_property(DiscreteMultiPropertyMonitor &monitor, const int time, const std::vector<std::pair<std::string_view, bool>> &propositionInputs) {
    db_interval_set::swapBuffers(monitor.holder);
    run_evaluation(monitor.nodes, monitor.proposition_map, monitor.holder, time, propositionInputs);
    monitor.outputs.clear();
    for (int rootIdx : monitor.propertyRootNodeIndexes) {
        monitor.outputs.push_back(monitor.nodes[rootIdx].output);
    }
    return monitor.outputs;
}

DenseMultiPropertyMonitor createDenseMultiPropertyMonitor(unsigned int holder_size) {
    DenseMultiPropertyMonitor monitor;
    monitor.holder = db_interval_set::newHolder(holder_size);
    return monitor;
}

void finalize_monitor(DenseMultiPropertyMonitor &monitor, std::vector<std::string> proposition_names_in_input_order) {
    // Remap proposition leftOperandIndex to match the user's input order
    for (size_t i = 0; i < proposition_names_in_input_order.size(); i++) {
        auto it = monitor.proposition_map.find(proposition_names_in_input_order[i]);
        if (it != monitor.proposition_map.end()) {
            monitor.nodes[it->second].leftOperandIndex = static_cast<unsigned int>(i);
        }
    }
    monitor.outputs.reserve(monitor.propertyCount);
    monitor.finalized = true;
}

void finalize_monitor(DenseMultiPropertyMonitor &monitor) {
    monitor.outputs.reserve(monitor.propertyCount);
    monitor.finalized = true;
}

const std::vector<db_interval_set::IntervalSet> &eval_multi_property(DenseMultiPropertyMonitor &monitor, const int startTime, const int endTime, const std::vector<bool> &propositionInputs) {
    db_interval_set::swapBuffers(monitor.holder);
    run_evaluation(monitor.nodes, monitor.holder, startTime, endTime, propositionInputs);
    monitor.outputs.clear();
    for (int rootIdx : monitor.propertyRootNodeIndexes) {
        monitor.outputs.push_back(monitor.nodes[rootIdx].output);
    }
    return monitor.outputs;
}


db_interval_set::IntervalSet run_evaluation(std::vector<DenseNode> &nodes, std::map<std::string, unsigned int, std::less<>> &proposition_map, db_interval_set::IntervalSetHolder &setHolder, const int startTime, const int endTime, const std::vector<std::pair<std::string_view, bool>> &propositionInputs) {
    for(const auto &propInput: propositionInputs) {
        auto &propNode = nodes[proposition_map.find(propInput.first)->second];
        if (propInput.second) propNode.output = db_interval_set::fromInterval(setHolder, {startTime, endTime});
        else propNode.output = db_interval_set::empty(setHolder);
    }
    
    int nodeCount = nodes.size();
    for(size_t node_index = 0; node_index < nodeCount; node_index++) {
        DenseNode &curNode = nodes[node_index];
        switch (curNode.type)
        {          
        case NodeType::PROPOSITION:
            break;
        case NodeType::AND:
            curNode.output = db_interval_set::intersectSets(setHolder, nodes[curNode.leftOperandIndex].output, nodes[curNode.rightOperandIndex].output);
            break;
        case NodeType::OR:
            curNode.output = db_interval_set::unionSets(setHolder, nodes[curNode.leftOperandIndex].output, nodes[curNode.rightOperandIndex].output);
            break;
        case NodeType::NOT:
            curNode.output = db_interval_set::negateSet(setHolder, nodes[curNode.rightOperandIndex].output, {startTime, endTime});
            break;    
        case NodeType::IMPLIES:
        {
            // A IMPLIES B is equivalent to (NOT A) OR B
            auto notLeft = db_interval_set::negateSet(setHolder, nodes[curNode.leftOperandIndex].output,
                                                                {startTime, endTime});
            
            auto right = nodes[curNode.rightOperandIndex].output;
            curNode.output = db_interval_set::unionSets(setHolder, notLeft, right);
            break;
        } 
        
        case NodeType::EVENTUALLY:
            {
            auto rightOutput = nodes[curNode.rightOperandIndex].output;

            curNode.output = db_interval_set::empty(setHolder);

            auto iterator = db_interval_set::createSegmentIterator(db_interval_set::empty(setHolder), rightOutput, {startTime, endTime});
            int i = 0;
            while (db_interval_set::getNextSegment(iterator)) {
                if (iterator.interval.end == iterator.interval.start) continue;
                if (iterator.rightTruthy) {
                    curNode.state = db_interval_set::unionSets(setHolder, curNode.state,
                        db_interval_set::fromInterval(setHolder, {iterator.interval.start + curNode.a, add_with_inf(iterator.interval.end, curNode.b)}));
                }
                i++;
                

                auto segmentOutput = db_interval_set::intersectSets(setHolder, curNode.state,
                    db_interval_set::fromInterval(setHolder, {iterator.interval.start, iterator.interval.end}));
                curNode.output = db_interval_set::unionSets(setHolder, curNode.output, segmentOutput);
                
            }
            curNode.state = db_interval_set::intersectSets(setHolder, curNode.state,
                db_interval_set::fromInterval(setHolder, {endTime, B_INFINITY}));
            break;
        }
        case NodeType::ALWAYS:
            {
            auto rightOutput = nodes[curNode.rightOperandIndex].output;

            curNode.output = db_interval_set::empty(setHolder);

            auto iterator = db_interval_set::createSegmentIterator(db_interval_set::empty(setHolder), rightOutput, {startTime, endTime});
            
            while (db_interval_set::getNextSegment(iterator)) {
                if (iterator.interval.end == iterator.interval.start) continue;
                if (!iterator.rightTruthy) {
                    curNode.state = db_interval_set::unionSets(setHolder, curNode.state,
                        db_interval_set::fromInterval(setHolder, {iterator.interval.start + curNode.a, add_with_inf(iterator.interval.end, curNode.b)}));
                }

                auto segmentOutput = db_interval_set::negateSet(setHolder, curNode.state, {iterator.interval.start, iterator.interval.end});
                curNode.output = db_interval_set::unionSets(setHolder, curNode.output, segmentOutput);
                
            }
            curNode.state = db_interval_set::intersectSets(setHolder, curNode.state,
                db_interval_set::fromInterval(setHolder, {endTime, B_INFINITY}));
            break;
        }
        case NodeType::SINCE:
            {
            auto leftOutput = nodes[curNode.leftOperandIndex].output;
            auto rightOutput = nodes[curNode.rightOperandIndex].output;
            curNode.output = db_interval_set::empty(setHolder);

            auto iterator = db_interval_set::createSegmentIterator(leftOutput, rightOutput, {startTime, endTime});
            
            while (db_interval_set::getNextSegment(iterator)) {
                if (iterator.interval.end == iterator.interval.start) continue;
                if (iterator.leftTruthy && iterator.rightTruthy) {
                    curNode.state = db_interval_set::unionSets(setHolder, curNode.state,
                        db_interval_set::fromInterval(setHolder, {iterator.interval.start + curNode.a, add_with_inf(iterator.interval.end, curNode.b)}));
                }
                else if (!iterator.leftTruthy && iterator.rightTruthy) {
                    curNode.state = db_interval_set::fromInterval(setHolder, {iterator.interval.end + curNode.a, add_with_inf(iterator.interval.end, curNode.b)});
                }
                else if (iterator.leftTruthy && !iterator.rightTruthy) {
                }
                else {
                    curNode.state = db_interval_set::empty(setHolder);
                }

                auto segmentOutput = db_interval_set::intersectSets(setHolder, curNode.state,
                    db_interval_set::fromInterval(setHolder, {iterator.interval.start, iterator.interval.end}));
                curNode.output = db_interval_set::unionSets(setHolder, curNode.output, segmentOutput);
                
            }
            curNode.state = db_interval_set::intersectSets(setHolder, curNode.state,
                db_interval_set::fromInterval(setHolder, {endTime, B_INFINITY}));
            break;
            }
        case NodeType::TEST:
            break;
        }
    
    }
    return nodes[nodeCount - 1].output;
}



const std::vector<db_interval_set::IntervalSet> &eval_multi_property(DenseMultiPropertyMonitor &monitor, const TimescalesInput &input) {
    db_interval_set::swapBuffers(monitor.holder);
    run_evaluation(monitor.nodes, monitor.proposition_map, monitor.holder, input.startTime, input.endTime, input.propositionInputs);
    monitor.outputs.clear();
    for (int rootIdx : monitor.propertyRootNodeIndexes) {
        monitor.outputs.push_back(monitor.nodes[rootIdx].output);
    }
    return monitor.outputs;
}


} // namespace loomrv
