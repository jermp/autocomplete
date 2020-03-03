#pragma once

#include "bit_vector.hpp"
#include "darray.hpp"
#include "util_types.hpp"

namespace autocomplete {
namespace ef {

struct ef_sequence {
    ef_sequence()
        : m_l(0)
        , m_size(0) {}

    template <typename T>
    void build(std::vector<T> const& from) {
        compress(from.begin(), from.size(), from.back());
    }

    template <typename T, typename Pointers>
    void build(std::vector<T> const& from, Pointers const& pointers) {
        build(from.begin(), from.size(), pointers);
    }

    template <typename Iterator, typename Pointers>
    void build(Iterator begin, uint64_t n, Pointers const& pointers) {
        std::vector<uint64_t> values;
        values.reserve(n);
        uint64_t prev_upper = 0;
        auto pointers_it = pointers.begin();
        uint64_t start = *pointers_it;
        ++pointers_it;
        uint64_t end = *pointers_it;
        uint64_t run = end - start;
        uint64_t within = 0;
        for (uint64_t i = 0; i < n; ++i, ++begin) {
            if (within == run) {
                within = 0;
                do {
                    start = end;
                    ++pointers_it;
                    end = *pointers_it;
                    run = end - start;
                } while (!run);
                prev_upper = values.size() ? values.back() : 0;
            }
            uint64_t v = *begin;
            values.push_back(v + prev_upper);
            ++within;
        }
        assert(values.size() == n);
        compress(values.begin(), values.size(), values.back());
    }

    struct iterator {
        iterator() {}

        iterator(ef_sequence const& ef, uint64_t pos = 0)
            : m_ef(&ef)
            , m_val(m_ef->universe())
            , m_pos(pos)
            , m_l(ef.m_l) {
            assert(pos < m_ef->size());
            m_low_mask = (uint64_t(1) << m_l) - 1;
            m_low_buf = 0;

            if (m_l) {
                m_chunks_in_word = 64 / m_l;
                m_low_buf = m_ef->m_low_bits.get_word64(m_pos * m_l);
                m_chunks_avail = m_chunks_in_word - 1;
            } else {
                m_chunks_in_word = 0;
                m_chunks_avail = m_ef->num_ones();
            }

            if (!m_ef->num_ones()) return;
            uint64_t begin =
                m_ef->m_high_bits_d1.select(m_ef->m_high_bits, m_pos);
            m_high_enum = bit_vector::unary_iterator(m_ef->m_high_bits, begin);
            m_high = m_high_enum.next();
            assert(m_l < 64);
        }

        uint64_t next() {
            if (m_pos == m_ef->size()) return m_val;
            if (!m_chunks_avail--) {
                m_low_buf = m_ef->m_low_bits.get_word64(m_pos * m_l);
                m_chunks_avail = m_chunks_in_word - 1;
            }
            assert(m_high ==
                   m_ef->m_high_bits_d1.select(m_ef->m_high_bits, m_pos));
            uint64_t low = m_low_buf & m_low_mask;
            m_val = (((m_high - m_pos) << m_l) | low);
            ++m_pos;
            m_low_buf >>= m_l;
            m_high = m_high_enum.next();
            return m_val;
        }

        uint64_t operator*() {
            if (m_pos == m_ef->size()) return m_val;
            assert(m_high ==
                   m_ef->m_high_bits_d1.select(m_ef->m_high_bits, m_pos));
            uint64_t low = m_low_buf & m_low_mask;
            m_val = (((m_high - m_pos) << m_l) | low);
            return m_val;
        }

        void operator++() {
            ++m_pos;
            if (m_pos == m_ef->size()) return;
            m_high = m_high_enum.next();
            m_low_buf >>= m_l;
            if (!m_chunks_avail--) {
                m_low_buf = m_ef->m_low_bits.get_word64(m_pos * m_l);
                m_chunks_avail = m_chunks_in_word - 1;
            }
        }

    private:
        ef_sequence const* m_ef;
        uint64_t m_val;
        uint64_t m_pos;
        uint64_t m_l;
        uint64_t m_high;

        bit_vector::unary_iterator m_high_enum;
        uint64_t m_low_buf;
        uint64_t m_low_mask;
        uint64_t m_chunks_in_word;
        uint64_t m_chunks_avail;
    };

    inline uint64_t access(uint64_t i) const {
        assert(i < size());
        return ((m_high_bits_d1.select(m_high_bits, i) - i) << m_l) |
               m_low_bits.get_bits(i * m_l, m_l);
    }

    inline uint64_t num_ones() const {
        return m_high_bits_d1.num_positions();
    }

    uint64_t find(const range r, uint64_t id) const {
        assert(r.is_valid());
        assert(r.end <= size());
        uint64_t prev_upper = previous_range_upperbound(r);
        return util::find(*this, id + prev_upper, r.begin, r.end - 1);
    }

    range find(const range r, const range lex) const {
        assert(r.is_valid());
        assert(r.end <= size());
        auto prev_upper = previous_range_upperbound(r);
        uint64_t id_begin = lex.begin + prev_upper;
        uint64_t id_end = lex.end + prev_upper;
        uint64_t begin = util::next_geq(*this, id_begin, r.begin, r.end - 1);
        if (begin == global::not_found or access(begin) > id_end) {
            return {r.end, r.end};
        }
        if (lex.begin == lex.end) return {begin, begin + 1};
        uint64_t end = util::next_geq(*this, id_end, begin, r.end - 1);
        if (end == global::not_found) return {begin, r.end};
        return {begin, access(end) != id_end ? end : end + 1};
    }

    inline range operator[](uint64_t i) const {
        return {access(i), access(i + 1)};
    }

    inline uint64_t size() const {
        return m_size;
    }

    inline uint64_t universe() const {
        return access(m_size - 1);
    }

    iterator begin() const {
        return iterator(*this);
    }

    iterator end() const {
        return iterator(*this, size());
    }

    iterator at(uint64_t pos) const {
        return iterator(*this, pos);
    }

    uint64_t bytes() const {
        return sizeof(m_l) + m_high_bits.bytes() + m_high_bits_d1.bytes() +
               m_low_bits.bytes() + sizeof(m_size);
    }

    void swap(ef_sequence& other) {
        std::swap(other.m_size, m_size);
        other.m_high_bits.swap(m_high_bits);
        other.m_high_bits_d1.swap(m_high_bits_d1);
        other.m_low_bits.swap(m_low_bits);
        std::swap(other.m_l, m_l);
    }

    template <typename Visitor>
    void visit(Visitor& visitor) {
        visitor.visit(m_l);
        visitor.visit(m_high_bits);
        visitor.visit(m_high_bits_d1);
        visitor.visit(m_low_bits);
        visitor.visit(m_size);
    }

private:
    uint8_t m_l;
    bit_vector m_high_bits;
    darray1 m_high_bits_d1;
    bit_vector m_low_bits;
    uint64_t m_size;

    template <typename Iterator>
    void compress(Iterator begin, uint64_t n, uint64_t u) {
        m_l = uint8_t((n && u / n) ? util::msb(u / n) : 0);
        m_size = n;
        bit_vector_builder bvb_high_bits(n + (u >> m_l) + 1);
        bit_vector_builder bvb_low_bits;
        bvb_low_bits.reserve(n * m_l);

        uint64_t low_mask = (uint64_t(1) << m_l) - 1;
        uint64_t last = 0;
        for (size_t i = 0; i < n; ++i, ++begin) {
            auto v = *begin;
            if (i and v < last) {
                throw std::runtime_error("sequence is not sorted.");
            }
            if (m_l) bvb_low_bits.append_bits(v & low_mask, m_l);
            bvb_high_bits.set((v >> m_l) + i, 1);
            last = v;
        }

        bit_vector(&bvb_high_bits).swap(m_high_bits);
        bit_vector(&bvb_low_bits).swap(m_low_bits);
        darray1(m_high_bits).swap(m_high_bits_d1);
    }

    uint64_t previous_range_upperbound(const range r) const {
        assert(r.is_valid());
        return r.begin ? access(r.begin - 1) : 0;
    }
};
}  // namespace ef
}  // namespace autocomplete