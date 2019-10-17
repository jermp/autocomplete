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
    const float c = 0.01;

    {
        // build, print and write
        ef_blocked_inverted_index::builder builder(params, c);
        ef_blocked_inverted_index bii;
        builder.build(bii);
        std::cout << "using " << bii.bytes() << " bytes" << std::endl;
        std::cout << "num docs " << bii.num_docs() << std::endl;
        std::cout << "num terms " << bii.num_terms() << std::endl;
    }

    return 0;
}
