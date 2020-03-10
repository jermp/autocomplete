#include <iostream>

#include "types.hpp"
#include "benchmark_common.hpp"

using namespace autocomplete;

template <typename Dictionary>
void perf_test(Dictionary const& dict,
               std::vector<std::string> const& queries) {
    static std::vector<uint8_t> decoded(2 * constants::MAX_NUM_CHARS_PER_QUERY);
    essentials::timer_type timer;

    for (uint32_t i = 0; i != benchmarking::runs; ++i) {
        timer.start();
        for (auto const& query : queries) {
            id_type id = dict.locate(string_to_byte_range(query));
            essentials::do_not_optimize_away(id);
        }
        timer.stop();
    }

    std::cout << "locate: " << timer.average() / queries.size()
              << " [musec/string]" << std::endl;

    std::vector<id_type> ids;
    ids.reserve(queries.size());
    for (auto const& query : queries) {
        id_type id = dict.locate(string_to_byte_range(query));
        ids.push_back(id);
    }

    timer.reset();

    for (uint32_t i = 0; i != benchmarking::runs; ++i) {
        timer.start();
        for (auto const& id : ids) {
            uint8_t string_len = dict.extract(id, decoded.data());
            essentials::do_not_optimize_away(string_len);
        }
        timer.stop();
    }

    std::cout << "extract: " << timer.average() / ids.size()
              << " [musec/string]" << std::endl;

    static std::vector<float> percentages = {0.0, 0.25, 0.50, 0.75, 1.0};
    for (auto p : percentages) {
        timer.reset();
        for (uint32_t i = 0; i != benchmarking::runs; ++i) {
            timer.start();
            for (auto const& query : queries) {
                size_t size = query.size();
                size_t n = size * p;
                if (n == 0) n += 1;  // at least one char
                uint8_t const* addr =
                    reinterpret_cast<uint8_t const*>(query.data());
                range r = dict.locate_prefix({addr, addr + n});
                essentials::do_not_optimize_away(r.end - r.begin);
            }
            timer.stop();
        }

        std::cout << "\tlocate_prefix-" << p * 100.0
                  << "%: " << timer.average() / queries.size()
                  << " [musec/string]" << std::endl;
    }
}

#define exe(BUCKET_SIZE)                                                     \
    {                                                                        \
        fc_dictionary<BUCKET_SIZE, uint32_vec> dict;                         \
        {                                                                    \
            fc_dictionary<BUCKET_SIZE, uint32_vec>::builder builder(params); \
            builder.build(dict);                                             \
            std::cout << "using " << dict.bytes() << " bytes" << std::endl;  \
        }                                                                    \
        perf_test<fc_dictionary<BUCKET_SIZE, uint32_vec>>(dict, queries);    \
    }

int main(int argc, char** argv) {
    cmd_line_parser::parser parser(argc, argv);
    parser.add("collection_basename", "Collection basename.");
    parser.add("max_num_queries", "Maximum number of queries to execute.");
    if (!parser.parse()) return 1;

    parameters params;
    params.collection_basename = parser.get<std::string>("collection_basename");
    params.load();

    auto max_num_queries = parser.get<uint32_t>("max_num_queries");

    essentials::logger("loading queries...");
    std::vector<std::string> queries;
    queries.reserve(max_num_queries);
    std::string query;
    query.reserve(2 * constants::MAX_NUM_CHARS_PER_QUERY);
    for (uint32_t i = 0; i != max_num_queries; ++i) {
        if (!std::getline(std::cin, query)) break;
        queries.push_back(std::move(query));
    }
    max_num_queries = queries.size();
    essentials::logger("loaded " + std::to_string(max_num_queries) +
                       " queries");

    exe(4) exe(8) exe(16) exe(32) exe(64) exe(128) exe(256) return 0;
}