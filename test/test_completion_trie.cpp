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
        uint32_completion_trie::builder builder(params);
        uint32_completion_trie ct;
        builder.build(ct);

        std::cout << "using " << ct.bytes() << " bytes" << std::endl;
        ct.print();

        if (output_filename) {
            // essentials::print_size(ct);
            essentials::logger("saving data structure to disk...");
            essentials::save<uint32_completion_trie>(ct, output_filename);
            essentials::logger("DONE");
        }
    }

    {
        // load and print
        uint32_completion_trie ct;
        essentials::logger("loading data structure from disk...");
        essentials::load(ct, output_filename);
        essentials::logger("DONE");
        // essentials::print_size(ct);
        std::cout << "using " << ct.bytes() << " bytes" << std::endl;
        ct.print();

        // test prefix_range() for all prefixes
        std::ifstream input(params.collection_basename, std::ios_base::in);
        completion_iterator it(params, input);
        while (input) {
            completion const& c = *it;
            for (uint32_t prefix_len = 1; prefix_len <= c.size();
                 ++prefix_len) {
                completion prefix(prefix_len);
                for (uint32_t i = 0; i != prefix_len; ++i) {
                    prefix.push_back(c[i]);
                }
                range r = ct.prefix_range(prefix);
                std::cout << "prefix range of '" << prefix << "' is ["
                          << r.begin << "," << r.end << ")" << std::endl;
            }
            ++it;
        }
    }

    return 0;
}
