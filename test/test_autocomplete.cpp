#include <iostream>

#include "types.hpp"

using namespace autocomplete;

int main(int argc, char** argv) {
    int mandatory = 2;
    if (argc < mandatory) {
        std::cout << argv[0] << " <collection_basename> [-o output_filename]"
                  << std::endl;
        return 1;
    }

    char const* output_filename = nullptr;

    for (int i = mandatory; i != argc; ++i) {
        if (std::string(argv[i]) == "-o") {
            ++i;
            output_filename = argv[i];
        }
    }

    parameters params;
    params.collection_basename = argv[1];
    params.load();

    {
        // build, print and write
        autocomplete_type1 a(params);
        std::cout << "using " << a.bytes() << " bytes" << std::endl;

        if (output_filename) {
            // essentials::print_size(a);
            essentials::logger("saving data structure to disk...");
            essentials::save<autocomplete_type1>(a, output_filename);
            essentials::logger("DONE");
        }
    }

    {
        // load and print
        autocomplete_type1 a;
        essentials::logger("loading data structure from disk...");
        essentials::load(a, output_filename);
        essentials::logger("DONE");
        // essentials::print_size(a);
        std::cout << "using " << a.bytes() << " bytes" << std::endl;

        uint32_t k = 5;
        std::string query("a");  // 10
        auto it = a.prefix_topk(query, k);
        std::cout << "size " << it.size() << std::endl;
        for (uint32_t i = 0; i != it.size(); ++i, ++it) {
            byte_range br = *it;
            print(br);
        }
    }

    return 0;
}
