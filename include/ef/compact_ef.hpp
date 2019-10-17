#pragma once

#include <stdexcept>

#include "ef_parameters.hpp"
#include "util.hpp"
#include "bit_vector.hpp"

namespace autocomplete {
namespace ef {

struct compact_ef {
    static constexpr bool is_byte_aligned = false;

    template <typename Iterator>
    static void build(bit_vector_builder& bvb, Iterator begin,
                      uint64_t universe, uint64_t n) {
        using util::ceil_div;
        ef_parameters params;
        uint64_t base_offset = bvb.size();
        ef_offsets of(base_offset, universe, n, params);
        // initialize all the bits to 0
        bvb.zero_extend(of.end - base_offset);

        uint64_t sample1_mask = (uint64_t(1) << of.log_sampling1) - 1;
        uint64_t offset;

        // utility function to set 0 pointers
        auto set_ptr0s = [&](uint64_t begin, uint64_t end, uint64_t rank_end) {
            uint64_t begin_zeros = begin - rank_end;
            uint64_t end_zeros = end - rank_end;

            for (uint64_t ptr0 =
                     ceil_div(begin_zeros, uint64_t(1) << of.log_sampling0);
                 (ptr0 << of.log_sampling0) < end_zeros; ++ptr0) {
                if (!ptr0) continue;
                offset = of.pointers0_offset + (ptr0 - 1) * of.pointer_size;
                assert(offset + of.pointer_size <= of.pointers1_offset);
                bvb.set_bits(offset, (ptr0 << of.log_sampling0) + rank_end,
                             of.pointer_size);
            }
        };

        uint64_t last = 0;
        uint64_t last_high = 0;
        Iterator it = begin;
        for (size_t i = 0; i < n; ++i) {
            uint64_t v = *it++;
            if (i and v < last) {
                throw std::runtime_error("sequence is not sorted");
            }
            assert(v <= universe);

            uint64_t high = (v >> of.lower_bits) + i + 1;
            uint64_t low = v & of.mask;

            bvb.set(of.higher_bits_offset + high, 1);

            offset = of.lower_bits_offset + i * of.lower_bits;
            assert(offset + of.lower_bits <= of.end);
            bvb.set_bits(offset, low, of.lower_bits);

            if (i and (i & sample1_mask) == 0) {
                uint64_t ptr1 = i >> of.log_sampling1;
                assert(ptr1 > 0);
                offset = of.pointers1_offset + (ptr1 - 1) * of.pointer_size;
                assert(offset + of.pointer_size <= of.higher_bits_offset);
                bvb.set_bits(offset, high, of.pointer_size);
            }

            // write pointers for the run of zeros in [last_high, high)
            set_ptr0s(last_high + 1, high, i);
            last_high = high;
            last = v;
        }

        // pointers to zeros after the last 1
        set_ptr0s(last_high + 1, of.higher_bits_length, n);  // XXX
    }

    struct iterator {
        static const uint64_t linear_scan_threshold = 8;

        iterator() {}

        iterator(bit_vector const& bv, uint64_t offset, uint64_t universe,
                 uint64_t n)
            : m_bv(&bv)
            , m_position(0) {
            ef_parameters params;
            m_of = ef_offsets(offset, universe, n, params);
            m_value = access(m_position);
        }

        bool has_next() const {
            return m_position < size();
        }

        void operator++() {
            m_position += 1;
            assert(m_position <= size());
            if (LIKELY(m_position < size())) {
                m_value = read_next();
            } else {
                m_value = m_of.universe;
            }
        }

        uint64_t operator*() const {
            return m_value;
        }

        uint64_t access(uint64_t position) {
            assert(position <= size());
            uint64_t skip = position - m_position;
            if (LIKELY(position > m_position &&
                       skip <= linear_scan_threshold)) {
                m_position = position;
                if (UNLIKELY(m_position == size())) {
                    m_value = m_of.universe;
                } else {
                    bit_vector::unary_iterator he = m_high_enumerator;
                    for (size_t i = 0; i < skip; ++i) {
                        he.next();
                    }
                    m_value = ((he.position() - m_of.higher_bits_offset -
                                m_position - 1)
                               << m_of.lower_bits) |
                              read_low();
                    m_high_enumerator = he;
                }
                return m_value;
            }
            return slow_access(position);
        }

        uint64_t next_geq_by_scan(uint64_t lower_bound) {
            assert(access(m_position) == m_value);
            while (m_position != size()) {
                assert(m_position < size());
                auto m_value = operator*();
                if (m_value >= lower_bound) return m_value;
                operator++();
            }
            assert(m_position == size());
            m_value = m_of.universe;
            return m_value;
        }

        uint64_t next_geq(uint64_t lower_bound) {
            uint64_t high_lower_bound = lower_bound >> m_of.lower_bits;
            uint64_t cur_high = m_value >> m_of.lower_bits;
            uint64_t high_diff = high_lower_bound - cur_high;

            if (LIKELY(lower_bound > m_value &&
                       high_diff <= linear_scan_threshold)) {
                next_reader next_value(*this, m_position + 1);
                uint64_t val;
                do {
                    m_position += 1;
                    if (LIKELY(m_position < size())) {
                        val = next_value();
                    } else {
                        val = m_of.universe;
                        break;
                    }
                } while (val < lower_bound);
                m_value = val;
                return m_value;
            } else {
                return slow_next_geq(lower_bound);
            }
        }

        uint64_t size() const {
            return m_of.n;
        }

        uint64_t next() {
            m_position += 1;
            assert(m_position <= size());
            if (LIKELY(m_position < size())) {
                m_value = read_next();
            } else {
                m_value = m_of.universe;
            }
            return m_value;
        }

        uint64_t prev_value() const {
            if (m_position == 0) return 0;
            uint64_t prev_high = 0;
            if (LIKELY(m_position < size())) {
                prev_high =
                    m_bv->predecessor1(m_high_enumerator.position() - 1);
            } else {
                prev_high = m_bv->predecessor1(m_of.lower_bits_offset - 1);
            }
            prev_high -= m_of.higher_bits_offset;
            uint64_t prev_pos = m_position - 1;
            uint64_t prev_low = m_bv->get_word56(m_of.lower_bits_offset +
                                                 prev_pos * m_of.lower_bits) &
                                m_of.mask;
            return ((prev_high - prev_pos - 1) << m_of.lower_bits) | prev_low;
        }

        uint64_t position() const {
            return m_position;
        }

        uint64_t value() const {
            return m_value;
        }

        bool intersects(const range r) {
            uint64_t val = access(0);
            if (val > r.end) return false;
            if (r.contains(val)) return true;
            for (uint64_t i = 1; i != size(); ++i) {
                auto val = next();
                if (val > r.end) break;
                if (r.contains(val)) return true;
            }
            return false;
        }

    private:
        uint64_t NOINLINE slow_access(uint64_t position) {
            if (UNLIKELY(position == size())) {
                m_position = position;
                m_value = m_of.universe;
                return m_value;
            }

            uint64_t skip = position - m_position;
            uint64_t to_skip;
            if (position > m_position && (skip >> m_of.log_sampling1) == 0) {
                to_skip = skip - 1;
            } else {
                uint64_t ptr = position >> m_of.log_sampling1;
                uint64_t high_pos = pointer1(ptr);
                uint64_t high_rank = ptr << m_of.log_sampling1;
                m_high_enumerator = bit_vector::unary_iterator(
                    *m_bv, m_of.higher_bits_offset + high_pos);
                to_skip = position - high_rank;
            }

            m_high_enumerator.skip(to_skip);
            m_position = position;
            m_value = read_next();
            return m_value;
        }

        uint64_t NOINLINE slow_next_geq(uint64_t lower_bound) {
            if (UNLIKELY(lower_bound >= m_of.universe)) {
                return access(size());
            }

            uint64_t high_lower_bound = lower_bound >> m_of.lower_bits;
            uint64_t cur_high = m_value >> m_of.lower_bits;
            uint64_t high_diff = high_lower_bound - cur_high;

            uint64_t to_skip;
            if (lower_bound > m_value &&
                (high_diff >> m_of.log_sampling0) == 0) {
                // NOTE: at the current position in the bitvector there
                // should be a 1, but since we already consumed it, it
                // is 0 in the enumerator, so we need to skip it
                to_skip = high_diff;
            } else {
                uint64_t ptr = high_lower_bound >> m_of.log_sampling0;
                uint64_t high_pos = pointer0(ptr);
                uint64_t high_rank0 = ptr << m_of.log_sampling0;

                m_high_enumerator = bit_vector::unary_iterator(
                    *m_bv, m_of.higher_bits_offset + high_pos);
                to_skip = high_lower_bound - high_rank0;
            }

            m_high_enumerator.skip0(to_skip);
            m_position = m_high_enumerator.position() -
                         m_of.higher_bits_offset - high_lower_bound;

            next_reader read_value(*this, m_position);
            while (true) {
                if (UNLIKELY(m_position == size())) {
                    m_value = m_of.universe;
                    return m_value;
                }
                auto val = read_value();
                if (val >= lower_bound) {
                    m_value = val;
                    return m_value;
                }
                m_position++;
            }
        }

        inline uint64_t read_low() {
            return m_bv->get_word56(m_of.lower_bits_offset +
                                    m_position * m_of.lower_bits) &
                   m_of.mask;
        }

        inline uint64_t read_next() {
            assert(m_position < size());
            uint64_t high = m_high_enumerator.next() - m_of.higher_bits_offset;
            return ((high - m_position - 1) << m_of.lower_bits) | read_low();
        }

        struct next_reader {
            next_reader(iterator& e, uint64_t position)
                : e(e)
                , high_enumerator(e.m_high_enumerator)
                , high_base(e.m_of.higher_bits_offset + position + 1)
                , lower_bits(e.m_of.lower_bits)
                , lower_base(e.m_of.lower_bits_offset + position * lower_bits)
                , mask(e.m_of.mask)
                , bv(*e.m_bv) {}

            ~next_reader() {
                e.m_high_enumerator = high_enumerator;
            }

            uint64_t operator()() {
                uint64_t high = high_enumerator.next() - high_base;
                uint64_t low = bv.get_word56(lower_base) & mask;
                high_base += 1;
                lower_base += lower_bits;
                return (high << lower_bits) | low;
            }

            iterator& e;
            bit_vector::unary_iterator high_enumerator;
            uint64_t high_base, lower_bits, lower_base, mask;
            bit_vector const& bv;
        };

        inline uint64_t pointer(uint64_t offset, uint64_t i) const {
            if (i == 0) {
                return 0;
            } else {
                return m_bv->get_word56(offset + (i - 1) * m_of.pointer_size) &
                       ((uint64_t(1) << m_of.pointer_size) - 1);
            }
        }

        inline uint64_t pointer0(uint64_t i) const {
            return pointer(m_of.pointers0_offset, i);
        }

        inline uint64_t pointer1(uint64_t i) const {
            return pointer(m_of.pointers1_offset, i);
        }

        bit_vector const* m_bv;
        ef_offsets m_of;

        uint64_t m_position;
        uint64_t m_value;
        bit_vector::unary_iterator m_high_enumerator;
    };
};
}  // namespace ef
}  // namespace autocomplete