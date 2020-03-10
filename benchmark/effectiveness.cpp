#include <iostream>

#include "types.hpp"
#include "benchmark_common.hpp"

using namespace autocomplete;

template <typename Index>
void benchmark(std::string const& index_filename, uint32_t k,
               uint32_t max_num_queries, float keep,
               essentials::json_lines& stats, bool verbose) {
    Index index1, index2;
    essentials::load(index1, index_filename.c_str());
    essentials::load(index2, index_filename.c_str());

    std::vector<std::string> queries;
    uint32_t num_queries =
        load_queries(queries, max_num_queries, keep, std::cin);
    uint64_t strings_reported_by_prefix_search = 0;
    uint64_t better_scored_strings_reported_by_conjunctive_search = 0;

    stats.add("num_queries", std::to_string(num_queries));

    std::vector<uint64_t> difference;
    difference.reserve(k);
    nop_probe probe;

    for (auto const& query : queries) {
        auto it1 = index1.prefix_topk(query, k, probe);
        auto it2 = index2.conjunctive_topk(query, k, probe);
        strings_reported_by_prefix_search += it1.size();

        uint64_t more = 0;
        assert(it2.size() >= it1.size());

        auto const& prefix_search_scores = it1.pool()->const_scores();
        auto const& conjunctive_search_scores = it2.pool()->const_scores();
        assert(std::is_sorted(prefix_search_scores.begin(),
                              prefix_search_scores.begin() + it1.size()));
        assert(std::is_sorted(conjunctive_search_scores.begin(),
                              conjunctive_search_scores.begin() + it2.size()));

        if (verbose) {
            std::cout << "query: '" << query << "'" << std::endl;
            {
                auto it = it1;
                std::cout << "prefix_search results: " << it.size()
                          << std::endl;
                for (uint64_t i = 0; i != it.size(); ++i, ++it) {
                    auto completion = *it;
                    std::cout << completion.score << ": "
                              << std::string(completion.string.begin,
                                             completion.string.end)
                              << std::endl;
                }
            }
            {
                auto it = it2;
                std::cout << "conjunctive_search results: " << it.size()
                          << std::endl;
                for (uint64_t i = 0; i != it.size(); ++i, ++it) {
                    auto completion = *it;
                    std::cout << completion.score << ": "
                              << std::string(completion.string.begin,
                                             completion.string.end)
                              << std::endl;
                }
            }
        }

        difference.clear();
        auto it = std::set_difference(
            conjunctive_search_scores.begin(),
            conjunctive_search_scores.begin() + it2.size(),
            prefix_search_scores.begin(),
            prefix_search_scores.begin() + it1.size(), difference.begin());
        more = std::distance(difference.begin(), it);
        if (verbose) std::cout << "more: " << more << std::endl;
        better_scored_strings_reported_by_conjunctive_search += more;
    }

    stats.add("strings_reported_by_prefix_search",
              std::to_string(strings_reported_by_prefix_search));
    stats.add(
        "better_scored_strings_reported_by_conjunctive_search",
        std::to_string(better_scored_strings_reported_by_conjunctive_search));
    stats.add(
        "better_scored_strings_reported_by_conjunctive_search_in_percentage",
        std::to_string(better_scored_strings_reported_by_conjunctive_search *
                       100.0 / strings_reported_by_prefix_search));
}

int main(int argc, char** argv) {
    cmd_line_parser::parser parser(argc, argv);
    parser.add("type", "Index type.");
    parser.add("k", "top-k value.");
    parser.add("index_filename", "Index filename.");
    parser.add("num_terms_per_query", "Number of terms per query.");
    parser.add("max_num_queries", "Maximum number of queries to execute.");
    parser.add("percentage",
               "A float in [0,1] specifying how much we keep of the last token "
               "in a query: n x 100 <=> n%, for n in [0,1].");
    parser.add("verbose", "Verbose output.", "--verbose");
    if (!parser.parse()) return 1;

    auto type = parser.get<std::string>("type");
    auto k = parser.get<uint32_t>("k");
    auto index_filename = parser.get<std::string>("index_filename");
    auto max_num_queries = parser.get<uint32_t>("max_num_queries");
    auto keep = parser.get<float>("percentage");
    auto verbose = parser.get<bool>("verbose");

    essentials::json_lines stats;
    stats.new_line();
    stats.add("num_terms_per_query",
              parser.get<std::string>("num_terms_per_query"));
    stats.add("percentage", std::to_string(keep));

    if (type == "ef_type1") {
        benchmark<ef_autocomplete_type1>(index_filename, k, max_num_queries,
                                         keep, stats, verbose);
    } else if (type == "ef_type2") {
        benchmark<ef_autocomplete_type2>(index_filename, k, max_num_queries,
                                         keep, stats, verbose);
    } else if (type == "ef_type3") {
        benchmark<ef_autocomplete_type3>(index_filename, k, max_num_queries,
                                         keep, stats, verbose);
    } else if (type == "ef_type4") {
        benchmark<ef_autocomplete_type4>(index_filename, k, max_num_queries,
                                         keep, stats, verbose);
    } else {
        return 1;
    }

    stats.print();
    return 0;
}