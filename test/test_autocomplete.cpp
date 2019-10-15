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

    // typedef uncompressed_autocomplete_type index_type;
    // typedef ef_autocomplete_type index_type;
    typedef ef_autocomplete_type2 index_type;

    {
        index_type index(params);
        if (output_filename) {
            essentials::logger("saving data structure to disk...");
            essentials::save<index_type>(index, output_filename);
            essentials::logger("DONE");
        }
    }

    {
        if (output_filename) {
            index_type index;
            essentials::logger("loading data structure from disk...");
            essentials::load(index, output_filename);
            essentials::logger("DONE");
            index.print_stats();

            {
                essentials::logger("testing prefix_topk()...");
                uint32_t k = 7;
                std::vector<std::string> queries = {
                    "a",        "10",          "african",
                    "air",      "commercial",  "internet",
                    "paris",    "somerset",    "the",
                    "the new",  "the perfect", "the starting line",
                    "yu gi oh", "for sale",    "dave mat",
                    "florence", "florida be",  "for s",
                    "for sa",   "for sal",     "for sale",
                    "ford a",   "ford au",     "ford m",
                    "ford mu",  "for",         "fo",
                    "f",        "matt",        "fl",
                    "florir",   "fly",         "the starting l",
                    "floridaaa"};

                for (auto& query : queries) {
                    auto it = index.prefix_topk(query, k);
                    std::cout << "top-" << it.size() << " completions for '"
                              << query << "':\n";
                    for (uint32_t i = 0; i != it.size(); ++i, ++it) {
                        auto completion = *it;
                        std::cout << "(" << completion.score << ", '";
                        print(completion.string);
                        std::cout << "')" << std::endl;
                    }
                }

                essentials::logger("DONE");
            }

            {
                essentials::logger("testing conjunctive_topk()...");
                uint32_t k = 7;
                std::vector<std::string> queries = {
                    "dave mat", "florence", "florida be",    "for s",
                    "for sa",   "for sal",  "for sale",      "ford a",
                    "ford au",  "ford m",   "ford mu",       "for",
                    "fo",       "f",        "matt",          "fl",
                    "flor",     "fly",      "the starting l"};

                for (auto& query : queries) {
                    auto it = index.conjunctive_topk(query, k);
                    std::cout << "top-" << it.size() << " completions for '"
                              << query << "':\n";
                    for (uint32_t i = 0; i != it.size(); ++i, ++it) {
                        auto completion = *it;
                        std::cout << "(" << completion.score << ", '";
                        print(completion.string);
                        std::cout << "')" << std::endl;
                    }
                }

                essentials::logger("DONE");
            }
        }
    }

    return 0;
}
