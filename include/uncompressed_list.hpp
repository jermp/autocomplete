#pragma once

#include <vector>

#include "util.hpp"
#include "bit_vector.hpp"

namespace autocomplete {

struct uncompressed_list {
    template <typename Iterator>
    static void build(bit_vector_builder& bvb, Iterator begin, uint64_t n) {
        for (uint64_t i = 0; i != n; ++i, ++begin) {
            bvb.append_bits(*begin, 32);
        }
    }

    struct iterator {
        iterator(bit_vector const& bv, uint64_t offset, uint64_t /*universe*/,
                 uint64_t n, compression_parameters const& /*params*/)
            : m_size(n)
            , m_position(0) {
            assert(offset % 32 == 0);
            m_data = reinterpret_cast<uint32_t const*>(bv.data().data()) +
                     offset / 32;
        }

        value_type move(uint64_t position) {
            assert(position < size());
            m_position = position;
            return {m_position, m_data[m_position]};
        }

        value_type next_geq(uint64_t lower_bound) {
            uint32_t lo = m_position;
            uint32_t hi = m_size;
            while (lo <= hi) {
                if (hi - lo <= global::linear_scan_threshold) {
                    for (m_position = lo; m_position != hi; ++m_position) {
                        uint32_t val = m_data[m_position];
                        if (val >= lower_bound) {
                            return {m_position, val};
                        }
                    }
                }
                uint32_t pos = (lo + hi) / 2;
                uint32_t val = m_data[pos];
                if (val == lower_bound) {
                    m_position = pos;
                    return {m_position, m_data[m_position]};
                } else if (val > lower_bound) {
                    hi = pos - 1;
                } else {
                    lo = pos + 1;
                }
            }
            assert(false);
            __builtin_unreachable();
        }

        value_type next() {
            m_position += 1;
            return {m_position, m_data[m_position]};
        }

        uint64_t size() const {
            return m_size;
        }

    private:
        uint64_t m_size;
        uint32_t m_position;
        uint32_t const* m_data;
    };
};

}  // namespace autocomplete