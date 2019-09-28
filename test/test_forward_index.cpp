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
        uncompressed_forward_index::builder builder(params);
        uncompressed_forward_index fi;
        builder.build(fi);
        std::cout << "using " << fi.bytes() << " bytes" << std::endl;
        std::cout << "num docs " << fi.num_docs() << std::endl;
        std::cout << "num terms " << fi.num_terms() << std::endl;

        if (output_filename) {
            essentials::logger("saving data structure to disk...");
            essentials::save<uncompressed_forward_index>(fi, output_filename);
            essentials::logger("DONE");
        }
    }

    {
        if (output_filename) {
            uncompressed_forward_index fi;
            essentials::logger("loading data structure from disk...");
            essentials::load(fi, output_filename);
            essentials::logger("DONE");
            std::cout << "using " << fi.bytes() << " bytes" << std::endl;
            std::cout << "num docs " << fi.num_docs() << std::endl;
            std::cout << "num terms " << fi.num_terms() << std::endl;
        }
    }

    return 0;
}
