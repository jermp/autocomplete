#pragma once

#include "util_types.hpp"

namespace autocomplete {

template <typename Dictionary>
uint32_t parse(Dictionary const& dict, std::string const& query,
               completion_type& prefix, byte_range& suffix) {
    uint32_t num_terms = 1;
    byte_range_iterator it(string_to_byte_range(query));
    while (true) {
        suffix = it.next();
        if (!it.has_next()) break;
        auto term_id = dict.locate(suffix);
        prefix.push_back(term_id);
        ++num_terms;
    }
    return num_terms;
}

}  // namespace autocomplete