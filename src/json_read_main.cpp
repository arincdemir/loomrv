#include <iostream>
#include <string_view>
#include "simdjson.h"

int main() {
    // 1. Load the NDJSON file
    simdjson::padded_string json_data;
    if (simdjson::padded_string::load("/home/arincdemir/workspace/loomrv/data/fullsuite/RespondBQR/Dense1/1M/RespondBQR10.jsonl").get(json_data)) {
        std::cerr << "Could not load file.\n";
        return 1;
    }

    simdjson::ondemand::parser parser;
    simdjson::ondemand::document_stream docs;

    if (parser.iterate_many(json_data).get(docs)) {
        std::cerr << "Failed to parse NDJSON.\n";
        return 1;
    }

    // 2. Stream through the file line by line
    for (auto doc : docs) {
        // Grab the current line as a JSON object
        simdjson::ondemand::object obj;
        if (doc.get_object().get(obj)) { 
            continue; // Skip safely if the line isn't a valid JSON object
        }

        std::cout << "Line Data: ";
        
        // 3. Iterate over every single key-value pair inside this specific object
        for (auto field : obj) {
            // Get the key name (unescaped_key returns a fast std::string_view)
            std::string_view key = field.unescaped_key();
            
            if (key == "time") {
                uint64_t time_val = 0;
                // field.value() accesses the data associated with this key
                if (!field.value().get(time_val)) {
                    std::cout << "[Time: " << time_val << "] ";
                }
            } else {
                // We found an unknown key! Read it as a boolean.
                bool dynamic_val = false;
                if (!field.value().get(dynamic_val)) {
                    std::cout << "{" << key << ": " << (dynamic_val ? "true" : "false") << "} ";
                }
            }
        }
        std::cout << "\n";
    }

    return 0;
}