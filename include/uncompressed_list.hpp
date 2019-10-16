#pragma once

#include "util.hpp"
#include "bit_vector.hpp"

namespace autocomplete {

template <typename UintType>
struct uncompressed_list {
    static constexpr uint8_t BITS = sizeof(UintType) * 8;
    static_assert(BITS <= 64, "bits per integer must be <= 64");
    static constexpr bool is_byte_aligned = true;

    template <typename Iterator>
    static void build(bit_vector_builder& bvb, Iterator begin,
                      uint64_t /*universe*/, uint64_t n) {
        assert(bvb.size() % 8 == 0);
        for (uint64_t i = 0; i != n; ++i, ++begin) {
            bvb.append_bits(*begin, BITS);
        }
    }

    struct iterator {
        iterator() {}

        iterator(bit_vector const& bv, uint64_t offset, uint64_t universe,
                 uint64_t n)
            : m_universe(universe)
            , m_size(n)
            , m_id(-1)
            , m_position(0) {
            assert(offset % 8 == 0);
            uint8_t const* base =
                reinterpret_cast<uint8_t const*>(bv.data().data()) + offset / 8;
            m_data = reinterpret_cast<UintType const*>(base);
        }

        bool has_next() const {
            return m_position < size();
        }

        void operator++() {
            m_position += 1;
        }

        UintType operator*() {
            m_id = m_data[m_position];
            return m_id;
        }

        UintType access(uint64_t i) {
            assert(i < size());
            m_position = i;
            m_id = m_data[m_position];
            return m_id;
        }

        UintType next_geq_by_scan(UintType lower_bound) {
            return next_geq_by_scan(m_position, size() - 1, lower_bound);
        }

        UintType next_geq(UintType lower_bound) {
            uint64_t lo = m_position;
            uint64_t hi = size() - 1;

            while (lo <= hi) {
                if (hi - lo <= global::linear_scan_threshold) {
                    return next_geq_by_scan(lo, hi, lower_bound);
                }

                m_position = (lo + hi) / 2;
                auto val = m_data[m_position];

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

        UintType next() {
            m_position += 1;
            m_id = m_position != size() ? m_data[m_position] : m_universe;
            return m_id;
        }

        bool intersects(const range r) const {
            for (uint64_t i = 0; i != size(); ++i) {
                auto val = m_data[i];
                if (val > r.end) break;
                if (r.contains(val)) return true;
            }
            return false;
        }

        uint64_t size() const {
            return m_size;
        }

        uint64_t position() const {
            return m_position;
        }

    private:
        uint64_t m_universe;
        uint64_t m_size;
        UintType m_id;
        uint64_t m_position;
        UintType const* m_data;

        UintType next_geq_by_scan(uint64_t lo, uint64_t hi,
                                  UintType lower_bound) {
            for (uint64_t i = lo; i <= hi; ++i) {
                auto val = m_data[i];
                if (val >= lower_bound) {
                    m_position = i;
                    m_id = val;
                    return m_id;
                }
            }
            m_position = hi + 1;
            m_id = m_universe;
            return m_id;
        }
    };
};

}  // namespace autocomplete