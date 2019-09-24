#pragma once

#include <vector>

#include "util.hpp"
#include "../external/essentials/include/essentials.hpp"

namespace autocomplete {

struct uint32_vec {
    void build(std::vector<uint32_t> const& from) {
        m_data = from;  // copy assignment operator
    }

    void build(std::vector<uint32_t> const& from,
               std::vector<uint32_t> const& pointers) {
        uint64_t n = from.size();
        m_data.reserve(n);
        uint64_t prev_upper = 0;
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
            uint64_t v = from[i];
            m_data.push_back(v + prev_upper);
            ++within;
        }

        assert(m_data.size() == n);
    }

    struct iterator {
        iterator(uint32_vec const& vec, uint64_t pos)
            : m_vec(&vec)
            , m_pos(pos) {}

        uint32_t operator*() {
            return m_vec->access(m_pos);
        }

        void operator++() {
            ++m_pos;
        }

    private:
        uint32_vec const* m_vec;
        uint64_t m_pos;
    };

    size_t size() const {
        return m_data.size();
    }

    uint32_t access(uint64_t i) const {
        assert(i < size());
        return m_data[i];
    }

    uint64_t find(range const& r, uint64_t id) const {
        assert(r.end > r.begin);
        assert(r.end <= size());
        uint64_t prev_upper = previous_range_upperbound(r);
        return scan_binary_search(*this, id + prev_upper, r.begin, r.end - 1);
    }

    inline range operator[](uint64_t i) const {
        return {access(i), access(i + 1)};
    }

    iterator at(uint64_t pos) const {
        return iterator(*this, pos);
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
    std::vector<uint32_t> m_data;

    uint64_t previous_range_upperbound(range const& r) const {
        return r.begin ? access(r.begin - 1) : 0;
    }
};

}  // namespace autocomplete