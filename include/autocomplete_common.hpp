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

uint32_t set_union(std::vector<id_type> const& l, const uint32_t size_l,
                   std::vector<id_type> const& r, const uint32_t size_r,
                   std::vector<id_type>& out, const uint32_t k) {
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t n = 0;

    while (i != size_l and j != size_r and n != k) {
        id_type x = l[i];
        id_type y = r[j];
        if (x == y) {
            out[n++] = x;
            ++i;
            ++j;
        } else if (x < y) {
            out[n++] = x;
            ++i;
        } else {
            out[n++] = y;
            ++j;
        }
    }

    for (; i != size_l and n != k; ++i, ++n) out[n] = l[i];
    for (; j != size_r and n != k; ++j, ++n) out[n] = r[j];

    assert(n <= k);

#ifdef DEBUG
    std::vector<id_type> expected_out(2 * constants::MAX_K);
    auto it = std::set_union(l.begin(), l.begin() + size_l, r.begin(),
                             r.begin() + size_r, expected_out.begin());
    uint32_t expected_n = std::distance(expected_out.begin(), it);
    if (expected_n > k) expected_n = k;
    assert(expected_n <= k);
    assert(n == expected_n);
    for (uint32_t i = 0; i != n; ++i) {
        if (out[i] != expected_out[i]) {
            throw std::runtime_error("mismatch");
        }
    }
#endif

    return n;
}

}  // namespace autocomplete