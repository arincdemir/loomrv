#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint> // For fixed width types (int32_t, etc.)
#include <string>

namespace binary_row_reader {

#pragma pack(push, 1)
struct TimescalesInput {
    int32_t time; // Matches 'i'
    bool p;       // Matches '?'
    bool q;       // Matches '?'
    bool r;       // Matches '?'
    bool s;       // Matches '?'
};
#pragma pack(pop)

std::vector<TimescalesInput> readInputFile(std::string fileName);

} // namespace binary_row_reader
