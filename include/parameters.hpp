#pragma once

#include <fstream>
#include <vector>
#include <cassert>

#include "constants.hpp"

namespace autocomplete {

struct parameters {
    parameters()
        : num_terms(0)
        , num_completions(0)
        , num_levels(0) {}

    void load() {
        std::ifstream input((collection_basename + ".mapped.stats").c_str(),
                            std::ios_base::in);
        if (!input.good()) {
            throw std::runtime_error("File with statistics not found");
        }
        input >> num_terms;
        input >> num_completions;
        input >> num_levels;
        assert(num_terms > 0);
        assert(num_completions > 0);
        assert(num_levels > 0);

        if (num_levels > constants::MAX_NUM_TERMS_PER_QUERY) {
            throw std::runtime_error(
                "Enlarge constants::MAX_NUM_TERMS_PER_QUERY");
        }

        nodes_per_level.resize(num_levels, 0);
        for (uint32_t i = 0; i != num_levels; ++i) {
            input >> nodes_per_level[i];
        }
    }

    uint32_t num_terms;
    uint32_t num_completions;
    uint32_t num_levels;
    std::vector<uint32_t> nodes_per_level;
    std::string collection_basename;
};

struct compression_parameters {
    compression_parameters()
        : ef_log_sampling0(9)
        , ef_log_sampling1(8)
        , rb_log_rank1_sampling(9)
        , rb_log_sampling1(8)
        , log_partition_size(7) {}

    uint8_t ef_log_sampling0;
    uint8_t ef_log_sampling1;
    uint8_t rb_log_rank1_sampling;
    uint8_t rb_log_sampling1;
    uint8_t log_partition_size;
};

}  // namespace autocomplete