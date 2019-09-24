#include <iostream>

#include "types.hpp"

using namespace autocomplete;

range prefix_range(std::vector<completion> const& completions,
                   completion const& c) {
    completion_comparator comp;
    auto b = std::lower_bound(completions.begin(), completions.end(), c, comp);
    uint64_t begin = std::distance(completions.begin(), b);
    auto e = std::upper_bound(completions.begin() + begin, completions.end(), c,
                              comp);
    uint64_t end = std::distance(completions.begin(), e);
    return {begin, end};
}

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
        // std::cout << "using " << ct.bytes() << " bytes" << std::endl;
        // ct.print();

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
        // ct.print();

        // test prefix_range() for all prefixes
        std::vector<completion> completions;
        completions.reserve(params.num_completions);
        std::ifstream input(params.collection_basename, std::ios_base::in);
        completion_iterator it(params, input);
        while (input) {
            completion const& c = *it;
            completions.push_back(std::move(c));
            ++it;
        }

        for (auto const& c : completions) {
            for (uint32_t prefix_len = 1; prefix_len <= c.size();
                 ++prefix_len) {
                completion prefix(prefix_len);
                for (uint32_t i = 0; i != prefix_len; ++i) {
                    prefix.push_back(c[i]);
                }
                range got = ct.prefix_range(prefix);
                range expected = prefix_range(completions, prefix);

                if ((got.begin != expected.begin) or
                    (got.end != expected.end)) {
                    std::cout << "Error: expected [" << expected.begin << ","
                              << expected.end << ") but got [" << got.begin
                              << "," << got.end << ")" << std::endl;
                    return 1;
                }

                std::cout << "prefix range of '" << prefix << "' is ["
                          << got.begin << "," << got.end << ")" << std::endl;
            }
        }
    }

    return 0;
}
