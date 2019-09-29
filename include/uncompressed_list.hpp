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
        iterator(bit_vector const& bv, uint64_t offset, uint64_t universe,
                 uint64_t n, compression_parameters const& /*params*/)
            : m_universe(universe)
            , m_size(n)
            , m_position(0) {
            assert(offset % 32 == 0);
            m_data = reinterpret_cast<uint32_t const*>(bv.data().data()) +
                     offset / 32;
        }

        uint32_t decode(uint32_t* out) {
            memcpy(out, m_data, size() * sizeof(uint32_t));
            return size();
        }

        id_type access(uint32_t i) {
            assert(i < size());
            m_position = i;
            m_id = m_data[m_position];
            return m_id;
        }

        id_type next_geq(id_type lower_bound) {
            uint32_t lo = m_position;
            uint32_t hi = size() - 1;

            while (lo <= hi) {
                if (hi - lo <= global::linear_scan_threshold) {
                    for (m_position = lo; m_position <= hi; ++m_position) {
                        uint32_t val = m_data[m_position];
                        if (val >= lower_bound) {
                            m_id = val;
                            return m_id;
                        }
                    }
                    break;
                }

                uint32_t m_position = (lo + hi) / 2;
                uint32_t val = m_data[m_position];

                if (val > lower_bound) {
                    hi = m_position != 0 ? m_position - 1 : 0;
                    if (lower_bound > m_data[hi]) {
                        m_id = val;
                        return m_id;
                    }
                } else if (val < lower_bound) {
                    lo = m_position + 1;
                } else {
                    m_id = val;
                    return m_id;
                }
            }

            m_id = m_universe;
            return m_id;
        }

        id_type next() {
            m_position += 1;
            m_id = m_position != size() ? m_data[m_position] : m_universe;
            return m_id;
        }

        bool contains(range r) const {
            for (uint32_t i = 0; i != size(); ++i) {
                uint32_t val = m_data[i];
                if (val > r.end) break;
                if (val >= r.begin and val <= r.end) return true;
            }
            return false;
        }

        // uint32_t position() const {
        //     return m_position;
        // }

        // uint64_t universe() const {
        //     return m_universe;
        // }

        uint64_t size() const {
            return m_size;
        }

    private:
        uint64_t m_universe;
        uint64_t m_size;
        id_type m_id;
        uint32_t m_position;
        uint32_t const* m_data;
    };
};

}  // namespace autocomplete