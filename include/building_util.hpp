#pragma once

#include "util.hpp"
#include "bit_vector.hpp"

namespace autocomplete {
namespace util {

std::vector<id_type> invert(std::vector<id_type> const& docid_to_lexid,
                            uint64_t size) {
    std::vector<id_type> lexid_to_docid(size);
    for (uint64_t doc_id = 0; doc_id != docid_to_lexid.size(); ++doc_id) {
        if (docid_to_lexid[doc_id] < size) {
            lexid_to_docid[docid_to_lexid[doc_id]] = doc_id;
        }
    }
    return lexid_to_docid;
}

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