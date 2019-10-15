#pragma once

#include "bit_vector.hpp"

namespace autocomplete {
namespace util {

void push_pad(bit_vector_builder& bvb, uint64_t alignment = 8) {
    uint64_t mod = bvb.size() % alignment;
    if (mod) {
        uint64_t pad = alignment - mod;
        bvb.append_bits(0, pad);
        assert(bvb.size() % alignment == 0);
    }
}

void eat_pad(bits_iterator<bit_vector>& it, uint64_t alignment = 8) {
    uint64_t mod = it.position() % alignment;
    if (mod) {
        uint64_t pad = alignment - mod;
        it.get_bits(pad);
        assert(it.position() % alignment == 0);
    }
}

}  // namespace util
}  // namespace autocomplete