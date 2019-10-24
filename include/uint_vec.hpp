#pragma once

#include <vector>

#include "util.hpp"

namespace autocomplete {

template <typename UintType>
struct uint_vec {
    template <typename T>
    void build(std::vector<T> const& from) {
        m_data.reserve(from.size());
        std::copy(from.begin(), from.end(), std::back_inserter(m_data));
    }

    template <typename T, typename Pointers>
    void build(std::vector<T> const& from, Pointers const& pointers) {
        uint64_t n = from.size();
        m_data.reserve(n);
        UintType prev_upper = 0;
        auto pointers_it = pointers.begin();
        uint64_t start = *pointers_it;
        ++pointers_it;
        uint64_t end = *pointers_it;
        uint64_t run = end - start;
        uint64_t within = 0;

        // compute in-range prefix-sums
        for (uint64_t i = 0; i != n; ++i) {
            if (within == run) {
                within = 0;
                do {
                    start = end;
                    ++pointers_it;
                    end = *pointers_it;
                    run = end - start;
                } while (!run);
                prev_upper = m_data.size() ? m_data.back() : 0;
            }
            UintType v = from[i];
            m_data.push_back(v + prev_upper);
            ++within;
        }

        assert(m_data.size() == n);
    }

    struct iterator {
        iterator(uint_vec<UintType> const* vec, uint64_t pos)
            : m_vec(vec)
            , m_pos(pos) {}

        UintType operator*() {
            return m_vec->access(m_pos);
        }

        void operator++() {
            ++m_pos;
        }

    private:
        uint_vec<UintType> const* m_vec;
        uint64_t m_pos;
    };

    size_t size() const {
        return m_data.size();
    }

    UintType access(uint64_t i) const {
        assert(i < size());
        return m_data[i];
    }

    uint64_t find(const range r, UintType id) const {
        assert(r.is_valid());
        assert(r.end <= size());
        auto prev_upper = previous_range_upperbound(r);
        return util::find(*this, id + prev_upper, r.begin, r.end - 1);
    }

    range find(const range r, const range lex) const {
        assert(r.is_valid());
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

    iterator at(uint64_t pos) const {
        return iterator(this, pos);
    }

    size_t bytes() const {
        return essentials::vec_bytes(m_data);
    }

    void print() const {
        for (auto x : m_data) {
            std::cout << x << " ";
        }
    }

    template <typename Visitor>
    void visit(Visitor& visitor) {
        visitor.visit(m_data);
    }

private:
    std::vector<UintType> m_data;

    UintType previous_range_upperbound(const range r) const {
        assert(r.is_valid());
        return r.begin ? access(r.begin - 1) : 0;
    }
};

}  // namespace autocomplete