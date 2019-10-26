#include <iostream>

#include "types.hpp"
#include "benchmark_common.hpp"

using namespace autocomplete;

template <typename Index>
void benchmark(std::string const& index_filename, uint32_t k,
               uint32_t max_num_queries, float keep,
               essentials::json_lines& breakdowns, bool breakdown) {
    Index index;
    essentials::load(index, index_filename.c_str());

    std::vector<std::string> queries;
    uint32_t num_queries =
        load_queries(queries, max_num_queries, keep, std::cin);

    uint64_t reported_strings = 0;
    auto musec_per_query = [&](double time) {
        return time / (runs * num_queries);
    };

    breakdowns.add("num_queries", std::to_string(num_queries));

    if (breakdown) {
        std::vector<timer_type> timers(4);
        for (uint32_t run = 0; run != runs; ++run) {
            for (auto const& query : queries) {
                auto it = index.topk(query, k, timers);
                reported_strings += it.size();
            }
        }
        std::cout << reported_strings << std::endl;
        breakdowns.add("parsing_musec_per_query",
                       std::to_string(musec_per_query(timers[0].elapsed())));
        breakdowns.add("prefix_search_musec_per_query",
                       std::to_string(musec_per_query(timers[1].elapsed())));
        breakdowns.add("conjunctive_search_musec_per_query",
                       std::to_string(musec_per_query(timers[2].elapsed())));
        breakdowns.add("reporting_musec_per_query",
                       std::to_string(musec_per_query(timers[3].elapsed())));
    } else {
        essentials::timer_type timer;
        timer.start();
        for (uint32_t run = 0; run != runs; ++run) {
            for (auto const& query : queries) {
                auto it = index.topk(query, k);
                reported_strings += it.size();
            }
        }
        timer.stop();
        std::cout << reported_strings << std::endl;
        breakdowns.add("musec_per_query",
                       std::to_string(musec_per_query(timer.elapsed())));
    }
}

int main(int argc, char** argv) {
    cmd_line_parser::parser parser(argc, argv);
    configure_parser_for_benchmarking(parser);
    if (!parser.parse()) return 1;

    auto type = parser.get<std::string>("type");
    auto k = parser.get<uint32_t>("k");
    auto index_filename = parser.get<std::string>("index_filename");
    auto max_num_queries = parser.get<uint32_t>("max_num_queries");
    auto keep = parser.get<float>("percentage");
    auto breakdown = parser.get<bool>("breakdown");

    essentials::json_lines breakdowns;
    breakdowns.new_line();
    breakdowns.add("num_terms_per_query",
                   parser.get<std::string>("num_terms_per_query"));
    breakdowns.add("percentage", std::to_string(keep));

    if (type == "ef_type1") {
        benchmark<ef_autocomplete_type1>(index_filename, k, max_num_queries,
                                         keep, breakdowns, breakdown);
    } else if (type == "ef_type2") {
        benchmark<ef_autocomplete_type2>(index_filename, k, max_num_queries,
                                         keep, breakdowns, breakdown);
    } else if (type == "ef_type3") {
        benchmark<ef_autocomplete_type3>(index_filename, k, max_num_queries,
                                         keep, breakdowns, breakdown);
    } else if (type == "ef_type4") {
        benchmark<ef_autocomplete_type4>(index_filename, k, max_num_queries,
                                         keep, breakdowns, breakdown);
    } else {
        return 1;
    }

    breakdowns.print();
    return 0;
}