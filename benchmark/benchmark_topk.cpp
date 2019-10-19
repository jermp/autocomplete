#include <iostream>

#include "types.hpp"
#include "statistics.hpp"
#include "benchmark_common.hpp"

using namespace autocomplete;

template <typename Index>
void benchmark_topk(char const* binary_filename, uint32_t k,
                    uint32_t max_num_queries, float keep,
                    essentials::json_lines& breakdowns) {
    Index index;
    // essentials::logger("loading data structure from disk...");
    essentials::load(index, binary_filename);
    // essentials::logger("DONE");
    // index.print_stats();

    std::vector<std::string> queries;
    // essentials::logger("loading queries...");
    uint32_t num_queries =
        load_queries(queries, max_num_queries, keep, std::cin);
    // essentials::logger("loaded " + std::to_string(num_queries) + " queries");

    // essentials::logger("benchmarking topk queries...");
    essentials::timer_type timer;
    uint64_t reported_strings = 0;

    timer.start();
    for (uint32_t run = 0; run != runs; ++run) {
        for (auto const& query : queries) {
            auto it = index.topk(query, k);
            reported_strings += it.size();
        }
    }
    timer.stop();

    // essentials::logger("DONE");
    std::cout << reported_strings << std::endl;

    auto musec_per_query = [&](double time) {
        return uint64_t(time / (runs * num_queries));
    };

    breakdowns.add("num_queries", std::to_string(num_queries));
    breakdowns.add("ns_per_query",
                   std::to_string(musec_per_query(timer.elapsed())));
}

int main(int argc, char** argv) {
    int mandatory = 6;
    if (argc < mandatory + 1) {
        std::cout << argv[0]
                  << " <type> <k> <binary_filename> <num_terms_per_query> "
                     "<max_num_queries> <percentage> < queries"
                  << std::endl;
        std::cout << "<percentage> is a float in [0,1] and specifies how much "
                     "we keep of the last token in a query "
                  << std::endl;
        return 1;
    }

    std::string type(argv[1]);
    uint32_t k = std::atoi(argv[2]);
    char const* binary_filename = argv[3];
    std::string num_terms_per_query(argv[4]);
    uint32_t max_num_queries = std::atoi(argv[5]);
    float keep = std::atof(argv[6]);

    essentials::json_lines breakdowns;
    breakdowns.new_line();
    breakdowns.add("num_terms_per_query", num_terms_per_query);
    breakdowns.add("percentage", std::to_string(keep));

    if (type == "ef_type1") {
        benchmark_topk<ef_autocomplete_type1>(
            binary_filename, k, max_num_queries, keep, breakdowns);
    } else if (type == "ef_type2") {
        benchmark_topk<ef_autocomplete_type2>(
            binary_filename, k, max_num_queries, keep, breakdowns);
    } else if (type == "ef_type3") {
        benchmark_topk<ef_autocomplete_type3>(
            binary_filename, k, max_num_queries, keep, breakdowns);
    } else if (type == "ef_type4") {
        benchmark_topk<ef_autocomplete_type4>(
            binary_filename, k, max_num_queries, keep, breakdowns);
    } else {
        return 1;
    }

    breakdowns.print();
    return 0;
}