#include <iostream>

#include "types.hpp"
#include "statistics.hpp"
#include "benchmark_common.hpp"

using namespace autocomplete;

typedef std::pair<completion_type, range> query_type;

template <typename Index>
void benchmark(parameters const& params, std::vector<query_type>& queries,
               uint32_t num_queries, uint32_t num_terms_per_query, float keep) {
    essentials::json_lines result;
    result.new_line();
    result.add("num_terms_per_query", std::to_string(num_terms_per_query));
    result.add("percentage", std::to_string(keep));
    result.add("num_queries", std::to_string(num_queries));

    Index index;
    {
        typename Index::builder builder(params);
        builder.build(index);
    }

    result.add("MiB", std::to_string(static_cast<double>(index.bytes()) /
                                     essentials::MiB));
    result.add(
        "bytes_per_completion",
        std::to_string(static_cast<double>(index.bytes()) / index.size()));

    essentials::timer_type timer;
    timer.start();
    for (uint32_t run = 0; run != benchmarking::runs; ++run) {
        for (auto& query : queries) {
            auto r = index.locate_prefix(query.first, query.second);
            essentials::do_not_optimize_away(r.end - r.begin);
        }
    }
    timer.stop();
    result.add(
        "musec_per_query",
        std::to_string(timer.elapsed() / (benchmarking::runs * num_queries)));
    result.print();
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
    auto num_terms_per_query = parser.get<uint32_t>("num_terms_per_query");
    auto keep = parser.get<float>("percentage");

    fc_dictionary_type dict;
    {
        fc_dictionary_type::builder builder(params);
        builder.build(dict);
    }

    std::vector<std::string> strings;
    std::vector<query_type> queries;
    uint32_t num_queries = 0;

    {
        num_queries = load_queries(strings, max_num_queries, keep, std::cin);
        for (auto const& string : strings) {
            completion_type prefix;
            byte_range suffix;
            parse(dict, string, prefix, suffix, true);
            range suffix_lex_range = dict.locate_prefix(suffix);
            queries.emplace_back(prefix, suffix_lex_range);
        }
    }

    if (type == "trie") {
        benchmark<ef_completion_trie>(params, queries, num_queries,
                                      num_terms_per_query, keep);
    } else if (type == "fc") {
        // benchmark<integer_fc_dictionary<4>>(params, queries, num_queries,
        //                                     num_terms_per_query, keep);
        // benchmark<integer_fc_dictionary<8>>(params, queries, num_queries,
        //                                     num_terms_per_query, keep);
        benchmark<integer_fc_dictionary<16>>(params, queries, num_queries,
                                             num_terms_per_query, keep);
        // benchmark<integer_fc_dictionary<32>>(params, queries, num_queries,
        //                                      num_terms_per_query, keep);
        // benchmark<integer_fc_dictionary<64>>(params, queries, num_queries,
        //                                      num_terms_per_query, keep);
        // benchmark<integer_fc_dictionary<128>>(params, queries, num_queries,
        //                                       num_terms_per_query, keep);
        // benchmark<integer_fc_dictionary<256>>(params, queries, num_queries,
        //                                       num_terms_per_query, keep);
    } else {
        return 1;
    }

    return 0;
}