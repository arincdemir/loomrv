#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cctype>

namespace json_reader {

struct TimescalesInput {
    int time;
    std::vector<bool> propositions;
};

TimescalesInput read_line(std::string &line);

} // namespace json_reader
