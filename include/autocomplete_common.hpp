#pragma once

#include "util_types.hpp"

namespace autocomplete {

template <typename Dictionary>
uint32_t parse(Dictionary const& dict, std::string& query,
               completion_type& prefix, byte_range& suffix) {
    uint32_t num_terms = parse_query(query);
    assert(num_terms > 0);
    prefix.reserve(num_terms);
    forward_byte_range_iterator it(string_to_byte_range(query));
    for (uint32_t i = 0; i != num_terms; ++i) {
        suffix = it.next();
        if (i == num_terms - 1) break;
        id_type term_id = dict.locate(suffix);
        prefix.push_back(term_id);
    }
    return num_terms;
}

}  // namespace autocomplete