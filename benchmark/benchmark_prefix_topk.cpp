#include <iostream>

#include "types.hpp"
#include "statistics.hpp"

using namespace autocomplete;
static const uint32_t runs = 10;

size_t load_queries(std::vector<std::string>& queries, uint32_t max_num_queries,
                    std::istream& is = std::cin) {
    std::string query;
    uint32_t num_queries = 0;
    std::cin >> num_queries;
    num_queries = std::min<uint32_t>(num_queries, max_num_queries);
    queries.reserve(num_queries);
    for (uint32_t i = 0; i != num_queries; ++i) {
        id_type doc_id;
        is >> doc_id;
        (void)doc_id;  // discard
        is >> query;
        queries.push_back(query);
    }
    return queries.size();
}

int main(int argc, char** argv) {
    int mandatory = 3;
    if (argc < mandatory + 1) {
        std::cout << argv[0]
                  << " <k> <binary_filename> <max_num_queries> < queries"
                  << std::endl;
        return 1;
    }

    uint32_t k = std::atoi(argv[1]);
    char const* binary_filename = argv[2];
    uint32_t max_num_queries = std::atoi(argv[3]);

    uncompressed_autocomplete_type autocomp;
    essentials::logger("loading data structure from disk...");
    essentials::load(autocomp, binary_filename);
    essentials::logger("DONE");
    autocomp.print_stats();

    std::vector<std::string> queries;
    essentials::logger("loading queries...");
    uint32_t num_queries = load_queries(queries, max_num_queries);
    essentials::logger("loaded " + std::to_string(num_queries) + " queries");

    essentials::logger("benchmarking prefix_topk queries...");
    std::vector<timer_type> timers(4);
    for (uint32_t run = 0; run != runs; ++run) {
        for (auto& query : queries) {
            autocomp.prefix_topk(query, k, timers);
        }
    }
    essentials::logger("DONE");

    auto ns_x_query = [&](double time) {
        return time / (runs * num_queries) * 1000.0;
    };

    std::cout << "prefix_topk timing breakdowns:\n";
    std::cout << "  parsing: ";
    std::cout << ns_x_query(timers[0].elapsed()) << " [ns x query]"
              << std::endl;
    std::cout << "  searching into completion trie: ";
    std::cout << ns_x_query(timers[1].elapsed()) << " [ns x query]"
              << std::endl;
    std::cout << "  topk RMQ: ";
    std::cout << ns_x_query(timers[2].elapsed()) << " [ns x query]"
              << std::endl;
    std::cout << "  reporting strings: ";
    std::cout << ns_x_query(timers[3].elapsed()) << " [ns x query]"
              << std::endl;

    return 0;
}