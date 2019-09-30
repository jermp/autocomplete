#include <iostream>

#include "types.hpp"
#include "statistics.hpp"

using namespace autocomplete;

int main(int argc, char** argv) {
    int mandatory = 1;
    if (argc < mandatory + 1) {
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

    uncompressed_autocomplete_type autocomp(params);
    autocomp.print_stats();

    if (output_filename) {
        essentials::logger("saving data structure to disk...");
        essentials::save<uncompressed_autocomplete_type>(autocomp,
                                                         output_filename);
        essentials::logger("DONE");
    }
}