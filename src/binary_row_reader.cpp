#include "loomrv/binary_row_reader.hpp"

namespace binary_row_reader {

std::vector<TimescalesInput> readInputFile(std::string fileName) {

    // 2. Open File
    // std::ios::ate is NOT used here because we need to read from the start
    std::ifstream file(fileName, std::ios::binary);

    // 3. Read the Header (The Count)
    uint32_t total_lines = 0;
    file.read(reinterpret_cast<char*>(&total_lines), sizeof(total_lines));

    // 4. Allocate Memory
    // We resize the vector to hold exactly the number of records.
    // This performs a single heap allocation.
    std::vector<TimescalesInput> data;
    data.resize(total_lines);

    // 5. Bulk Read (The "Cast" into Memory)
    // We calculate the total bytes needed: count * 8 bytes
    // We read directly from disk into the vector's internal array.
    long bytes_to_read = total_lines * sizeof(TimescalesInput);
    file.read(reinterpret_cast<char*>(data.data()), bytes_to_read);

    file.close();


    return data;
}

} // namespace binary_row_reader
