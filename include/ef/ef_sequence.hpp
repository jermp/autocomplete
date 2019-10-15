#pragma once

#include "bit_vector.hpp"
#include "darray.hpp"
#include "util_types.hpp"

namespace autocomplete {
namespace ef {

struct ef_sequence {
    ef_sequence()
        : m_l(0)
        , m_index_for_find(false)
        , m_size(0) {}

    template <typename T>
    void build(std::vector<T> const& from, bool index_for_find = false) {
        build(from.begin(), from.size(), from.back(), index_for_find);
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
        build(values.begin(), values.size(), values.back(), false);
    }

    struct iterator {
        iterator() {}

        iterator(ef_sequence const& ef, range r = {0, 0}, uint64_t pos = 0)
            : m_ef(&ef)
            , m_val(m_ef->universe())
            , m_pos(pos)
            , m_l(ef.m_l)
            , m_prev_range_upper_bound(ef.previous_range_upperbound(r)) {
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

        uint64_t value() {
            m_last = operator*();
            return m_last - m_prev_range_upper_bound;
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

        uint64_t m_prev_range_upper_bound;
        uint64_t m_last;

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

    inline uint64_t access(const range r, uint64_t pos) {
        return access(pos) - previous_range_upperbound(r);
    }

    inline uint64_t num_ones() const {
        return m_high_bits_d1.num_positions();
    }

    // inline uint64_t next_geq(uint64_t x) const {
    //     assert(m_high_bits_d0.num_positions());

    //     if (UNLIKELY(x >= universe())) {
    //         return size() - (x == universe());
    //     }

    //     uint64_t h_x = x >> m_l;
    //     uint64_t begin =
    //         h_x ? m_high_bits_d0.select(m_high_bits, h_x - 1) - h_x + 1 : 0;
    //     assert(begin < size());

    //     // uint64_t end = m_high_bits_d0.select(m_high_bits, h_x) - h_x;
    //     // assert(end <= size());
    //     // assert(begin <= end);
    //     // return binary search for x in [begin, end)

    //     auto it = at(begin);
    //     uint64_t pos = begin;
    //     uint64_t val = it.next();

    //     while (val < x) {
    //         ++pos;
    //         val = it.next();
    //     }

    //     return pos - (val != x);
    // }

    uint64_t find(const range r, uint64_t id) const {
        assert(!r.is_invalid());
        assert(r.end <= size());
        uint64_t prev_upper = previous_range_upperbound(r);
        return util::find(*this, id + prev_upper, r.begin, r.end - 1);
    }

    range find(const range r, const range lex) const {
        assert(!r.is_invalid());
        assert(r.end <= size());
        auto prev_upper = previous_range_upperbound(r);

        uint64_t begin =
            util::next_geq(*this, lex.begin + prev_upper, r.begin, r.end - 1);
        if (begin == global::not_found) {
            return {r.end, r.end};
        }

        if (lex.begin == lex.end) {
            return {begin, begin + 1};
        }

        uint64_t id_end = lex.end + prev_upper;
        uint64_t end = util::next_geq(*this, id_end, begin, r.end - 1);
        if (end == global::not_found) {
            return {begin, r.end};
        }

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
        return iterator(*this, {0, 0}, size());
    }

    iterator at(uint64_t pos) const {
        return iterator(*this, {0, 0}, pos);
    }

    iterator at(range const& r, uint64_t pos) const {
        return iterator(*this, r, pos);
    }

    uint64_t bytes() const {
        return sizeof(m_l) + sizeof(m_index_for_find) + m_high_bits.bytes() +
               m_high_bits_d1.bytes() +
               (m_index_for_find ? m_high_bits_d0.bytes() : 0) +
               m_low_bits.bytes() + sizeof(m_size);
    }

    void swap(ef_sequence& other) {
        std::swap(other.m_size, m_size);
        std::swap(other.m_index_for_find, m_index_for_find);
        other.m_high_bits.swap(m_high_bits);
        other.m_high_bits_d1.swap(m_high_bits_d1);
        other.m_high_bits_d0.swap(m_high_bits_d0);
        other.m_low_bits.swap(m_low_bits);
        std::swap(other.m_l, m_l);
    }

    template <typename Visitor>
    void visit(Visitor& visitor) {
        visitor.visit(m_l);
        visitor.visit(m_index_for_find);
        visitor.visit(m_high_bits);
        visitor.visit(m_high_bits_d1);

        if (m_index_for_find) {
            visitor.visit(m_high_bits_d0);
        }

        visitor.visit(m_low_bits);
        visitor.visit(m_size);
    }

private:
    uint8_t m_l;
    bool m_index_for_find;
    bit_vector m_high_bits;
    darray1 m_high_bits_d1;
    darray0 m_high_bits_d0;
    bit_vector m_low_bits;
    uint64_t m_size;

    template <typename Iterator>
    void build(Iterator begin, uint64_t n, uint64_t u, bool index_for_find) {
        m_l = uint8_t((n && u / n) ? util::msb(u / n) : 0);
        m_index_for_find = index_for_find;
        m_size = n;
        bit_vector_builder bvb_high_bits(n + (u >> m_l) + 1);
        bit_vector_builder bvb_low_bits;
        bvb_low_bits.reserve(n * m_l);

        uint64_t low_mask = (uint64_t(1) << m_l) - 1;
        uint64_t last = 0;
        for (size_t i = 0; i < n; ++i, ++begin) {
            auto v = *begin;
            if (i && v < last) {
                std::cout << "at pos: " << i << "/" << n << std::endl;
                std::cout << "v = " << v << "; last = " << last << std::endl;
                throw std::runtime_error("sequence is not sorted.");
            }
            if (m_l) {
                bvb_low_bits.append_bits(v & low_mask, m_l);
            }
            bvb_high_bits.set((v >> m_l) + i, 1);
            last = v;
        }

        bit_vector(&bvb_high_bits).swap(m_high_bits);
        bit_vector(&bvb_low_bits).swap(m_low_bits);
        darray1(m_high_bits).swap(m_high_bits_d1);

        if (index_for_find) {
            darray0(m_high_bits).swap(m_high_bits_d0);
        }
    }

    uint64_t previous_range_upperbound(const range r) const {
        assert(!r.is_invalid());
        return r.begin ? access(r.begin - 1) : 0;
    }
};
}  // namespace ef
}  // namespace autocomplete