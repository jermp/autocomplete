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
        uncompressed_inverted_index::builder builder(params);
        uncompressed_inverted_index ii;
        builder.build(ii);
        std::cout << "using " << ii.bytes() << " bytes" << std::endl;
        std::cout << "num docs " << ii.num_docs() << std::endl;
        std::cout << "num terms " << ii.num_terms() << std::endl;

        if (output_filename) {
            essentials::logger("saving data structure to disk...");
            essentials::save<uncompressed_inverted_index>(ii, output_filename);
            essentials::logger("DONE");
        }
    }

    {
        // load and print
        uncompressed_inverted_index ii;
        essentials::logger("loading data structure from disk...");
        essentials::load(ii, output_filename);
        essentials::logger("DONE");
        std::cout << "using " << ii.bytes() << " bytes" << std::endl;
        std::cout << "num docs " << ii.num_docs() << std::endl;
        std::cout << "num terms " << ii.num_terms() << std::endl;

        auto it = ii[0];
        for (uint32_t i = 0; i != it.size(); ++i) {
            auto val = it.move(i);
            std::cout << val.first << "," << val.second << std::endl;
        }
    }

    return 0;
}
