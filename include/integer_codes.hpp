#pragma once

#include "util.hpp"
#include "bit_vector.hpp"

namespace autocomplete {

void write_gamma(bit_vector_builder& bvb, uint64_t n) {
    uint64_t nn = n + 1;
    uint64_t l = util::msb(nn);
    uint64_t hb = uint64_t(1) << l;
    bvb.append_bits(hb, l + 1);
    bvb.append_bits(nn ^ hb, l);
}

void write_gamma_nonzero(bit_vector_builder& bvb, uint64_t n) {
    assert(n > 0);
    write_gamma(bvb, n - 1);
}

uint64_t read_gamma(bits_iterator<bit_vector>& it) {
    uint64_t l = it.skip_zeros();
    return (it.get_bits(l) | (uint64_t(1) << l)) - 1;
}

uint64_t read_gamma_nonzero(bits_iterator<bit_vector>& it) {
    return read_gamma(it) + 1;
}

void write_delta(bit_vector_builder& bvb, uint64_t n) {
    uint64_t nn = n + 1;
    uint64_t l = util::msb(nn);
    uint64_t hb = uint64_t(1) << l;
    write_gamma(bvb, l);
    bvb.append_bits(nn ^ hb, l);
}

uint64_t read_delta(bits_iterator<bit_vector>& it) {
    uint64_t l = read_gamma(it);
    return (it.get_bits(l) | (uint64_t(1) << l)) - 1;
}

}  // namespace autocomplete