#pragma once

#include "../external/cmd_line_parser/include/parser.hpp"
#include "probe.hpp"

namespace autocomplete {

namespace benchmarking {
static const uint32_t runs = 5;
}

// void tolower(std::string& str) {
//     std::transform(str.begin(), str.end(), str.begin(),
//                    [](unsigned char c) { return std::tolower(c); });
// }

size_t load_queries(std::vector<std::string>& queries, uint32_t max_num_queries,
                    float percentage, std::istream& is = std::cin) {
    assert(percentage >= 0.0 and percentage <= 1.0);
    std::string query;
    queries.reserve(max_num_queries);
    for (uint32_t i = 0; i != max_num_queries; ++i) {
        if (!std::getline(is, query)) break;
        assert(query.size() > 0);
        size_t size = query.size() - 1;
        while (size > 0 and query[size] != ' ') --size;
        size_t last_token_size = query.size() - size;
        size_t end = size + std::ceil(last_token_size * percentage) + 1 +
                     1;  // retain at least one char
        for (size = query.size(); size > end; --size) query.pop_back();
        // tolower(query);
        queries.push_back(query);
    }
    return queries.size();
}

void configure_parser_for_benchmarking(cmd_line_parser::parser& parser) {
    parser.add("type", "Index type.");
    parser.add("k", "top-k value.");
    parser.add("index_filename", "Index filename.");
    parser.add("num_terms_per_query", "Number of terms per query.");
    parser.add("max_num_queries", "Maximum number of queries to execute.");
    parser.add("percentage",
               "A float in [0,1] specifying how much we keep of the last token "
               "in a query: n x 100 <=> n%, for n in [0,1].");
}

#define BENCHMARK(what)                                                        \
    template <typename Index>                                                  \
    void benchmark(std::string const& index_filename, uint32_t k,              \
                   uint32_t max_num_queries, float keep,                       \
                   essentials::json_lines& breakdowns) {                       \
        Index index;                                                           \
        essentials::load(index, index_filename.c_str());                       \
                                                                               \
        std::vector<std::string> queries;                                      \
        uint32_t num_queries =                                                 \
            load_queries(queries, max_num_queries, keep, std::cin);            \
                                                                               \
        uint64_t reported_strings = 0;                                         \
        auto musec_per_query = [&](double time) {                              \
            return time / (benchmarking::runs * num_queries);                  \
        };                                                                     \
                                                                               \
        breakdowns.add("num_queries", std::to_string(num_queries));            \
                                                                               \
        timer_probe probe(3);                                                  \
        for (uint32_t run = 0; run != benchmarking::runs; ++run) {             \
            for (auto const& query : queries) {                                \
                auto it = index.what##topk(query, k, probe);                   \
                reported_strings += it.size();                                 \
            }                                                                  \
        }                                                                      \
        std::cout << "#ignore: " << reported_strings << std::endl;             \
                                                                               \
        breakdowns.add("reported_strings",                                     \
                       std::to_string(reported_strings / benchmarking::runs)); \
        breakdowns.add(                                                        \
            "parsing_musec_per_query",                                         \
            std::to_string(musec_per_query(probe.get(0).elapsed())));          \
        breakdowns.add(                                                        \
            std::string(#what) + "search_musec_per_query",                     \
            std::to_string(musec_per_query(probe.get(1).elapsed())));          \
        breakdowns.add(                                                        \
            "reporting_musec_per_query",                                       \
            std::to_string(musec_per_query(probe.get(2).elapsed())));          \
        breakdowns.add(                                                        \
            "total_musec_per_query",                                           \
            std::to_string(musec_per_query(probe.get(0).elapsed()) +           \
                           musec_per_query(probe.get(1).elapsed()) +           \
                           musec_per_query(probe.get(2).elapsed())));          \
    }                                                                          \
                                                                               \
    int main(int argc, char** argv) {                                          \
        cmd_line_parser::parser parser(argc, argv);                            \
        configure_parser_for_benchmarking(parser);                             \
        if (!parser.parse()) return 1;                                         \
                                                                               \
        auto type = parser.get<std::string>("type");                           \
        auto k = parser.get<uint32_t>("k");                                    \
        auto index_filename = parser.get<std::string>("index_filename");       \
        auto max_num_queries = parser.get<uint32_t>("max_num_queries");        \
        auto keep = parser.get<float>("percentage");                           \
                                                                               \
        essentials::json_lines breakdowns;                                     \
        breakdowns.new_line();                                                 \
        breakdowns.add("num_terms_per_query",                                  \
                       parser.get<std::string>("num_terms_per_query"));        \
        breakdowns.add("percentage", std::to_string(keep));                    \
                                                                               \
        if (type == "ef_type1") {                                              \
            benchmark<ef_autocomplete_type1>(                                  \
                index_filename, k, max_num_queries, keep, breakdowns);         \
        } else if (type == "ef_type2") {                                       \
            benchmark<ef_autocomplete_type2>(                                  \
                index_filename, k, max_num_queries, keep, breakdowns);         \
        } else if (type == "ef_type3") {                                       \
            benchmark<ef_autocomplete_type3>(                                  \
                index_filename, k, max_num_queries, keep, breakdowns);         \
        } else if (type == "ef_type4") {                                       \
            benchmark<ef_autocomplete_type4>(                                  \
                index_filename, k, max_num_queries, keep, breakdowns);         \
        } else {                                                               \
            return 1;                                                          \
        }                                                                      \
                                                                               \
        breakdowns.print();                                                    \
        return 0;                                                              \
    }

}  // namespace autocomplete