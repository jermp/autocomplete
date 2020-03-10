#include <iostream>
#include <algorithm>

#include "types.hpp"
#include "benchmark_common.hpp"

using namespace autocomplete;

template <typename Dictionary>
void perf_test(Dictionary const& dict, std::vector<id_type> const& queries) {
    static completion_type decoded(2 * constants::MAX_NUM_CHARS_PER_QUERY);
    essentials::timer_type timer;

    for (uint32_t i = 0; i != benchmarking::runs; ++i) {
        timer.start();
        for (auto const& id : queries) {
            uint8_t string_len = dict.extract(id, decoded);
            essentials::do_not_optimize_away(string_len);
        }
        timer.stop();
    }

    std::cout << "extract: " << (timer.average() * 1000.0) / queries.size()
              << " [ns/string]" << std::endl;
}

#define exe(BUCKET_SIZE)                                                     \
    {                                                                        \
        integer_fc_dictionary<BUCKET_SIZE, uint64_vec> dict;                 \
        {                                                                    \
            integer_fc_dictionary<BUCKET_SIZE, uint64_vec>::builder builder( \
                params);                                                     \
            builder.build(dict);                                             \
            std::cout << "using " << dict.bytes() << " bytes" << std::endl;  \
        }                                                                    \
        perf_test<integer_fc_dictionary<BUCKET_SIZE, uint64_vec>>(dict,      \
                                                                  queries);  \
    }

int main(int argc, char** argv) {
    int mandatory = 2 + 1;
    if (argc < mandatory) {
        std::cout << argv[0] << " <collection_basename> <num_queries>"
                  << std::endl;
        return 1;
    }

    parameters params;
    params.collection_basename = argv[1];
    params.load();

    uint32_t num_queries = std::atoi(argv[2]);
    std::vector<id_type> queries(num_queries);
    for (id_type i = 0; i != num_queries; ++i) queries[i] = i;
    const static uint32_t seed = 13;  // deterministic seed to replicate results
    std::shuffle(queries.begin(), queries.end(),
                 std::default_random_engine(seed));

    exe(4) exe(8) exe(16) exe(32) exe(64) exe(128) exe(256) return 0;
}