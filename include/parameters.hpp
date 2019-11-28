#pragma once

#include <fstream>
#include <vector>
#include <cassert>

#include "constants.hpp"

namespace autocomplete {

struct parameters {
    parameters()
        : num_terms(0)
        , max_string_length(0)
        , num_completions(0)
        , num_levels(0) {}

    void load() {
        std::ifstream input((collection_basename + ".mapped.stats").c_str(),
                            std::ios_base::in);
        if (!input.good()) {
            throw std::runtime_error("File with statistics not found");
        }
        input >> num_terms;
        input >> max_string_length;
        input >> num_completions;
        input >> universe;
        input >> num_levels;
        assert(num_terms > 0);
        assert(max_string_length > 0);
        assert(num_completions > 0);
        assert(universe >= num_completions);
        assert(num_levels > 0);

        if (max_string_length > constants::MAX_NUM_CHARS_PER_QUERY) {
            throw std::runtime_error(
                "Enlarge constants::MAX_NUM_CHARS_PER_QUERY");
        }

        if (num_levels > constants::MAX_NUM_TERMS_PER_QUERY) {
            throw std::runtime_error(
                "Enlarge constants::MAX_NUM_TERMS_PER_QUERY");
        }

        nodes_per_level.resize(num_levels, 0);
        uint32_t i = 0;
        for (; i != num_levels and input; ++i) input >> nodes_per_level[i];
        if (i != num_levels) {
            throw std::runtime_error(
                "File with statistics may be truncated or malformed");
        }
    }

    uint32_t num_terms;
    uint32_t max_string_length;
    uint32_t num_completions;
    uint32_t universe;
    uint32_t num_levels;
    std::vector<uint32_t> nodes_per_level;
    std::string collection_basename;
};

}  // namespace autocomplete