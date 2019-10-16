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

    typedef compact_forward_index forward_index_type;

    {
        forward_index_type::builder builder(params);
        forward_index_type index;
        builder.build(index);
        std::cout << "using " << index.bytes() << " bytes" << std::endl;
        std::cout << "num docs " << index.num_docs() << std::endl;
        std::cout << "num terms " << index.num_terms() << std::endl;

        if (output_filename) {
            essentials::logger("saving data structure to disk...");
            essentials::save<forward_index_type>(index, output_filename);
            essentials::logger("DONE");
        }
    }

    {
        if (output_filename) {
            forward_index_type index;
            essentials::logger("loading data structure from disk...");
            essentials::load(index, output_filename);
            essentials::logger("DONE");
            std::cout << "using " << index.bytes() << " bytes" << std::endl;
            std::cout << "num docs " << index.num_docs() << std::endl;
            std::cout << "num terms " << index.num_terms() << std::endl;
        }
    }

    return 0;
}
