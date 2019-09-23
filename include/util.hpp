#pragma once

namespace autocomplete {

// assume 32 bits are enough to store
// both a term_id and a doc_id
typedef uint32_t term_id_type;
typedef uint32_t doc_id_type;

namespace global {
static const doc_id_type invalid_doc_id = doc_id_type(-1);
static const term_id_type invalid_term_id = term_id_type(-1);
static const term_id_type terminator = 0;
static const uint64_t not_found = uint64_t(-1);
static const uint64_t linear_scan_threshold = 8;
}  // namespace global

template <typename S>
inline uint64_t scan_binary_search(S const& sequence, uint64_t id, uint64_t lo,
                                   uint64_t hi) {
    while (lo <= hi) {
        if (hi - lo <= global::linear_scan_threshold) {
            auto it = sequence.at(lo);
            for (uint64_t pos = lo; pos != hi; ++pos, ++it) {
                if (*it == id) {
                    return pos;
                }
            }
        }
        uint64_t pos = lo + ((hi - lo) >> 1);
        uint64_t val = sequence.access(pos);
        if (val == id) {
            return pos;
        } else if (val > id) {
            hi = pos - 1;
        } else {
            lo = pos + 1;
        }
    }
    return global::not_found;
}

}  // namespace autocomplete