#include <iostream>

#include "types.hpp"
#include "benchmark_common.hpp"

using namespace autocomplete;

template <typename Dictionary>
void perf_test(Dictionary const& dict,
               std::vector<std::string> const& queries) {
    static std::vector<uint8_t> decoded(2 * constants::MAX_NUM_CHARS_PER_QUERY);
    essentials::timer_type timer;

    for (uint32_t i = 0; i != runs; ++i) {
        timer.start();
        for (auto const& query : queries) {
            id_type id = dict.locate(string_to_byte_range(query));
            essentials::do_not_optimize_away(id);
        }
        timer.stop();
    }

    std::cout << "locate: " << (timer.average() * 1000.0) / queries.size()
              << " [ns/string]" << std::endl;

    std::vector<id_type> ids;
    ids.reserve(queries.size());
    for (auto const& query : queries) {
        id_type id = dict.locate(string_to_byte_range(query));
        ids.push_back(id);
    }

    timer.reset();

    for (uint32_t i = 0; i != runs; ++i) {
        timer.start();
        for (auto const& id : ids) {
            uint8_t string_len = dict.extract(id, decoded.data());
            essentials::do_not_optimize_away(string_len);
        }
        timer.stop();
    }

    std::cout << "extract: " << (timer.average() * 1000.0) / ids.size()
              << " [ns/string]" << std::endl;

    static std::vector<float> percentages = {0.0, 0.25, 0.50, 0.75, 1.0};
    // static std::vector<float> percentages = {0.1, 0.2, 0.3, 0.4, 0.5,
    //                                          0.6, 0.7, 0.8, 0.9, 1.0};
    for (auto p : percentages) {
        timer.reset();
        for (uint32_t i = 0; i != runs; ++i) {
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
                  << "%: " << (timer.average() * 1000.0) / queries.size()
                  << " [ns/string]" << std::endl;
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
    int mandatory = 2 + 1;
    if (argc < mandatory) {
        std::cout << argv[0] << " <collection_basename> <num_queries> < queries"
                  << std::endl;
        return 1;
    }

    parameters params;
    params.collection_basename = argv[1];
    params.load();

    uint32_t num_queries = std::atoi(argv[2]);

    essentials::logger("loading queries...");
    std::vector<std::string> queries;
    queries.reserve(num_queries);
    std::string query;
    query.reserve(2 * constants::MAX_NUM_CHARS_PER_QUERY);
    for (uint32_t i = 0; i != num_queries; ++i) {
        if (!std::getline(std::cin, query)) break;
        queries.push_back(std::move(query));
    }
    num_queries = queries.size();
    essentials::logger("loaded " + std::to_string(num_queries) + " queries");

    exe(4) exe(8) exe(16) exe(32) exe(64) exe(128) exe(256) return 0;
}