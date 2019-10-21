#include <iostream>

#include "types.hpp"
#include "statistics.hpp"
#include "benchmark_common.hpp"

using namespace autocomplete;

template <typename Index>
void benchmark_locate_prefix(parameters const& params,
                             fc_dictionary_type const& dict,
                             uint32_t max_num_queries, float keep,
                             essentials::json_lines& result) {
    Index index;
    {
        typename Index::builder builder(params);
        builder.build(index);
    }

    typedef std::pair<completion_type, range> query_type;
    std::vector<std::string> strings;
    std::vector<query_type> queries;
    uint32_t num_queries = 0;

    {
        num_queries = load_queries(strings, max_num_queries, keep, std::cin);
        for (auto const& string : strings) {
            completion_type prefix;
            byte_range suffix;
            parse(dict, string, prefix, suffix);
            range suffix_lex_range = dict.locate_prefix(suffix);
            queries.emplace_back(prefix, suffix_lex_range);
        }
    }

    auto musec_per_query = [&](double time) {
        return time / (runs * num_queries);
    };

    essentials::timer_type timer;
    timer.start();
    for (uint32_t run = 0; run != runs; ++run) {
        for (auto& query : queries) {
            auto r = index.locate_prefix(query.first, query.second);
            essentials::do_not_optimize_away(r.end - r.begin);
        }
    }
    timer.stop();
    result.add("musec_per_query",
               std::to_string(musec_per_query(timer.elapsed())));
}

int main(int argc, char** argv) {
    int mandatory = 5;
    if (argc < mandatory + 1) {
        std::cout << argv[0]
                  << " <type> <collection_basename> <num_terms_per_query> "
                     "<max_num_queries> <percentage> < queries"
                  << std::endl;
        std::cout << "<percentage> is a float in [0,1] and specifies how much "
                     "we keep of the last token in a query "
                  << std::endl;
        return 1;
    }

    std::string type(argv[1]);
    parameters params;
    params.collection_basename = argv[2];
    params.load();

    std::string num_terms_per_query(argv[3]);
    uint32_t max_num_queries = std::atoi(argv[4]);
    float keep = std::atof(argv[5]);

    fc_dictionary_type dict;
    {
        fc_dictionary_type::builder builder(params);
        builder.build(dict);
    }

    essentials::json_lines result;
    result.new_line();
    result.add("num_terms_per_query", num_terms_per_query);
    result.add("percentage", std::to_string(keep));

    if (type == "trie") {
        benchmark_locate_prefix<ef_completion_trie>(
            params, dict, max_num_queries, keep, result);
    } else if (type == "fc") {
        benchmark_locate_prefix<integer_fc_dictionary_type>(
            params, dict, max_num_queries, keep, result);
    } else {
        return 1;
    }

    result.print();
    return 0;
}