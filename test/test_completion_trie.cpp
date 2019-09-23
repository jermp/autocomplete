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

    parameters params;
    params.collection_basename = argv[1];
    params.load();

    uint32_completion_trie::builder builder(params);

    uint32_completion_trie ct;
    builder.build(ct);

    std::cout << "using " << ct.bytes() << " bytes" << std::endl;
    ct.print();

    char const* output_filename = nullptr;

    for (int i = mandatory; i != argc; ++i) {
        if (std::string(argv[i]) == "-o") {
            ++i;
            output_filename = argv[i];
        }
    }

    if (output_filename) {
        essentials::print_size(ct);
        essentials::logger("saving data structure to disk...");
        essentials::save<uint32_completion_trie>(ct, output_filename);
        essentials::logger("DONE");
    }

    return 0;
}
