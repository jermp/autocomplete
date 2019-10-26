#include <iostream>

#include "types.hpp"
#include "statistics.hpp"
#include "benchmark_common.hpp"

using namespace autocomplete;

template <typename Index>
void benchmark(parameters const& params, fc_dictionary_type const& dict,
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
        result.add("num_queries", std::to_string(num_queries));
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
    cmd_line_parser::parser parser(argc, argv);
    parser.add("type", "Index type.");
    parser.add("collection_basename", "Collection basename.");
    parser.add("num_terms_per_query", "Number of terms per query.");
    parser.add("max_num_queries", "Maximum number of queries to execute.");
    parser.add("percentage",
               "A float in [0,1] specifying how much we keep of the last token "
               "in a query.");
    if (!parser.parse()) return 1;

    parameters params;
    params.collection_basename = parser.get<std::string>("collection_basename");
    params.load();

    auto type = parser.get<std::string>("type");
    auto max_num_queries = parser.get<uint32_t>("max_num_queries");
    auto keep = parser.get<float>("percentage");

    fc_dictionary_type dict;
    {
        fc_dictionary_type::builder builder(params);
        builder.build(dict);
    }

    essentials::json_lines result;
    result.new_line();
    result.add("num_terms_per_query",
               parser.get<std::string>("num_terms_per_query"));
    result.add("percentage", std::to_string(keep));

    if (type == "trie") {
        benchmark<ef_completion_trie>(params, dict, max_num_queries, keep,
                                      result);
    } else if (type == "fc") {
        benchmark<integer_fc_dictionary_type>(params, dict, max_num_queries,
                                              keep, result);
    } else {
        return 1;
    }

    result.print();
    return 0;
}