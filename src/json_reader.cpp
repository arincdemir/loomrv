#include "loomrv/json_reader.hpp"

namespace json_reader {

TimescalesInput read_line(std::string &line) {
    std::vector<bool> propositions;
    int time = 0;
    std::string segment;
    size_t index = 0;
    while (!isdigit(line.at(index))) {
        index++;
    }
    size_t numStart = index - 1;
    while (isdigit(line.at(index))) {
        index++;
    }
    time = stoi(line.substr(numStart, index - numStart));
    
    while (line.at(index) != '}') {
        while (line.at(index) != ':') {
            index++;
        }
        size_t truthStart = index + 2;
        while (line.at(index) != ',' && line.at(index) != '}') {
            index++;
        }
        if ("true" == line.substr(truthStart, index - truthStart)) {
            propositions.push_back(true);
        }
        else {
            propositions.push_back(false);
        }
    }
    return TimescalesInput{time, propositions};

}

} // namespace json_reader
