#include "test_common.hpp"

using namespace autocomplete;

typedef ef_completion_trie completion_trie_type;

template <typename Dictionary, typename Index>
void test_locate_prefix(Dictionary const& dict, Index const& index,
                        std::vector<std::string> const& queries,
                        std::vector<std::string> const& strings) {
    for (auto const& query : queries) {
        range expected = testing::locate_prefix(strings, query);
        completion_type prefix;
        byte_range suffix;
        parse(dict, query, prefix, suffix, true);

        range suffix_lex_range = dict.locate_prefix(suffix);
        suffix_lex_range.begin += 1;
        suffix_lex_range.end += 1;
        range got = index.locate_prefix(prefix, suffix_lex_range);

        CHECK_MESSAGE((got.begin == expected.begin and got.end == expected.end),
                      "Error for query '"
                          << query << "': expected [" << expected.begin << ","
                          << expected.end << ") but got [" << got.begin << ","
                          << got.end << ")");
    }
}

TEST_CASE("test locate_prefix()") {
    parameters params;
    params.collection_basename = testing::test_filename.c_str();
    params.load();

    fc_dictionary_type dict;
    {
        fc_dictionary_type::builder builder(params);
        builder.build(dict);
    }

    std::vector<std::string> strings;

    {
        essentials::logger("loading all strings...");
        std::string line;
        strings.reserve(params.num_completions);
        std::ifstream input((params.collection_basename).c_str(),
                            std::ios_base::in);
        for (uint32_t i = 0; i != params.num_completions; ++i) {
            if (!std::getline(input, line)) break;
            auto s = line.substr(line.find(' ') + 1, line.size());
            strings.push_back(s);
        }
        input.close();
        essentials::logger("loaded " + std::to_string(strings.size()) +
                           " strings");
    }

    constexpr uint32_t max_num_queries = 5000;
    std::vector<std::string> queries;
    static std::vector<float> percentages = {0.0, 0.25, 0.50, 0.75, 1.0};
    static std::vector<uint32_t> query_terms = {1, 2, 3, 4, 5, 6, 7};

    completion_trie_type ct_index;
    integer_fc_dictionary_type fc_index;

    {
        completion_trie_type::builder builder(params);
        builder.build(ct_index);
        REQUIRE(ct_index.size() == params.num_completions);
    }

    {
        integer_fc_dictionary_type::builder builder(params);
        builder.build(fc_index);
        REQUIRE(fc_index.size() == params.num_completions);
    }

    for (auto perc : percentages) {
        for (auto num_terms : query_terms) {
            std::cout << "percentage " << perc * 100.0 << "%, num_terms "
                      << num_terms << std::endl;
            {
                queries.clear();
                std::string filename =
                    params.collection_basename +
                    ".queries/queries.length=" + std::to_string(num_terms);
                std::ifstream querylog(filename.c_str());
                if (!querylog.is_open()) {
                    std::cerr << "cannot open file '" << filename << "'"
                              << std::endl;
                    return;
                }
                load_queries(queries, max_num_queries, perc, querylog);
                querylog.close();
            }

            test_locate_prefix(dict, ct_index, queries, strings);
            test_locate_prefix(dict, fc_index, queries, strings);
        }
    }
}
