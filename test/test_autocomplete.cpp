#include <iostream>

#include "types.hpp"
#include "statistics.hpp"

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
        autocomplete_type1 a(params);
        if (output_filename) {
            essentials::logger("saving data structure to disk...");
            essentials::save<autocomplete_type1>(a, output_filename);
            essentials::logger("DONE");
        }
    }

    {
        if (output_filename) {
            autocomplete_type1 a;
            essentials::logger("loading data structure from disk...");
            essentials::load(a, output_filename);
            essentials::logger("DONE");
            a.print_stats();

            // test prefix_topk()
            {
                uint32_t k = 7;
                std::vector<std::string> queries = {
                    "a",        "10",          "african",
                    "air",      "commercial",  "internet",
                    "paris",    "somerset",    "the",
                    "the new",  "the perfect", "the starting line",
                    "yu gi oh", "for sale"};

                for (auto& query : queries) {
                    auto it = a.prefix_topk(query, k);
                    std::cout << "top-" << it.size() << " completions for '"
                              << query << "':\n";
                    for (uint32_t i = 0; i != it.size(); ++i, ++it) {
                        auto completion = *it;
                        std::cout << "(" << completion.score << ", '";
                        print(completion.string);
                        std::cout << "')" << std::endl;
                    }
                }
            }

            // test conjunctive_topk()
            {
                uint32_t k = 7;
                std::vector<std::string> queries = {
                    "dave mat", "florence", "florida be", "for s",   "for sa",
                    "for sal",  "for sale", "ford a",     "ford au", "ford m",
                    "ford mu",  "for",      "fo",         "f",       "matt",
                    "fl",       "flor",     "fly"};

                for (auto& query : queries) {
                    auto it = a.conjunctive_topk(query, k);
                    std::cout << "top-" << it.size() << " completions for '"
                              << query << "':\n";
                    for (uint32_t i = 0; i != it.size(); ++i, ++it) {
                        auto completion = *it;
                        std::cout << "(" << completion.score << ", '";
                        print(completion.string);
                        std::cout << "')" << std::endl;
                    }
                }
            }
        }
    }

    return 0;
}
