#include "test_common.hpp"

using namespace autocomplete;

typedef ef_autocomplete_type1 index_type;

TEST_CASE("test autocomplete topk functions") {
    char const* output_filename = testing::tmp_filename.c_str();
    parameters params;
    params.collection_basename = testing::test_filename.c_str();
    params.load();

    {
        index_type index(params);
        essentials::save<index_type>(index, output_filename);
    }

    {
        index_type index;
        essentials::load(index, output_filename);

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

            nop_probe probe;
            for (auto& query : queries) {
                auto it = index.prefix_topk(query, k, probe);
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

            nop_probe probe;
            for (auto& query : queries) {
                auto it = index.conjunctive_topk(query, k, probe);
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

    std::remove(output_filename);
}
