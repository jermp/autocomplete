#include <iostream>

#include "types.hpp"

using namespace autocomplete;

struct completion_comparator {
    bool operator()(completion_type const& lhs,
                    completion_type const& rhs) const {
        size_t l = 0;  // |lcp(lhs,rhs)|
        while (l < lhs.size() - 1 and l < rhs.size() - 1 and lhs[l] == rhs[l]) {
            ++l;
        }
        return lhs[l] < rhs[l];
    }
};

range prefix_range(std::vector<completion_type> const& completions,
                   completion_type const& c) {
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
        uint64_completion_trie::builder builder(params);
        uint64_completion_trie ct;
        builder.build(ct);

        if (output_filename) {
            // essentials::print_size(ct);
            essentials::logger("saving data structure to disk...");
            essentials::save<uint64_completion_trie>(ct, output_filename);
            essentials::logger("DONE");
        }
    }

    {
        if (output_filename) {
            uint64_completion_trie ct;
            essentials::logger("loading data structure from disk...");
            essentials::load(ct, output_filename);
            essentials::logger("DONE");
            // essentials::print_size(ct);
            std::cout << "using " << ct.bytes() << " bytes" << std::endl;

            std::vector<completion_type> completions;
            completions.reserve(params.num_completions);
            std::ifstream input(params.collection_basename + ".mapped",
                                std::ios_base::in);
            if (!input.good()) {
                throw std::runtime_error("File not found");
            }

            completion_iterator it(params, input);
            while (input) {
                auto& record = *it;
                completions.push_back(std::move(record.completion));
                ++it;
            }
            input.close();

            // check all completions
            essentials::logger("testing is_member()...");
            for (auto const& c : completions) {
                if (!ct.is_member(c)) {
                    print_completion(c);
                    std::cout << " not found!" << std::endl;
                    return 1;
                }
            }
            essentials::logger("DONE...");

            uint32_t num_checks =
                std::min<uint32_t>(params.num_completions, 30000);
            uint32_t check = 0;

            essentials::logger("testing prefix_range()...");
            completion_type prefix;
            for (auto const& c : completions) {
                for (uint32_t len = 1; len < c.size(); ++len) {
                    prefix.clear();
                    prefix.reserve(len + 1);
                    for (uint32_t i = 0; i != len; ++i) {
                        prefix.push_back(c[i]);
                    }

                    // std::cout << "prefix range of ";
                    // print_completion(prefix);

                    range got = ct.prefix_range(prefix);

                    // std::cout << "is [" << got.begin << "," << got.end << ")"
                    //           << std::endl;

                    range expected = prefix_range(completions, prefix);

                    if ((got.begin != expected.begin) or
                        (got.end != expected.end)) {
                        std::cout << "prefix range of ";
                        print_completion(prefix);
                        std::cout << std::endl;
                        std::cout << "Error: expected [" << expected.begin
                                  << "," << expected.end << ") but got ["
                                  << got.begin << "," << got.end << ")"
                                  << std::endl;
                        return 1;
                    }
                }

                check += 1;
                if (check == num_checks) break;
            }
            essentials::logger("DONE...");
        }
    }

    return 0;
}
