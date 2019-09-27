#pragma once

#include <vector>

#include "bp_vector.hpp"
#include "bp_vector_support.hpp"
#include "util.hpp"

namespace autocomplete {

// This class implements a cartesian-tree-based RMQ data
// structure, using the 2d-Min-Heap DFUDS representation described
// in "Space-Efficient Preprocessing Schemes for Range Minimum
// Queries on Static Arrays", Johannes Fischer and Volker Heun,
// SIAM J. Comput., 40(2), 465–492.

// We made a few variations:
//
// - The rmq() operation in the paper checks whether x is parent
//   of w - 1, which can be written as select0(x - 1) <
//   find_open(w - 1). We use instead the fact that the excess
//   between x and w (both excluded) is strictly greater than the
//   excess of w, so the formula above holds iff excess(select0(x
//   - 1) + 1) <= excess(w). This is faster because a select0 is
//   faster than find_open+rank0.
//
// - The construction is done in reverse order so that the input
//   array can be traversed left-to-right. This involves
//   re-mapping all the indices at query time. Since the array is
//   reversed, in ties the leftmost element wins
//
// - Our data structures have 0-based indices, so the operations
//   are slightly different from those in the paper

struct cartesian_tree {
    template <typename T>
    struct builder {
        builder(uint64_t expected_size = 0) {
            if (expected_size) {
                m_bp.reserve(2 * expected_size + 2);
            }
        }

        template <typename Comparator>
        void push_back(T const& val, Comparator const& comp) {
            m_bp.push_back(0);

            while (!m_stack.empty() &&
                   comp(val, m_stack.back())) {  // val < m_stack.back()
                m_stack.pop_back();
                m_bp.push_back(1);
            }

            m_stack.push_back(val);
        }

        bit_vector_builder& finalize() {
            // super-root
            m_bp.push_back(0);
            while (!m_stack.empty()) {
                m_stack.pop_back();
                m_bp.push_back(1);
            }
            m_bp.push_back(1);

            m_bp.reverse();
            return m_bp;
        }

        friend struct cartesian_tree;

    private:
        std::vector<T> m_stack;
        bit_vector_builder m_bp;
    };

    cartesian_tree() {}

    template <typename T, typename Comparator>
    void build(std::vector<T> const& v, Comparator const& comp) {
        builder<T> b;
        for (auto const& x : v) {
            b.push_back(x, comp);
        }
        cartesian_tree(&b).swap(*this);
    }

    // RMQ in the interval [a, b], b inclusive
    uint64_t rmq(uint64_t a, uint64_t b) const {
        typedef bp_vector::excess_t excess_t;

        assert(a <= b);
        if (a == b) return a;

        uint64_t n = size();

        uint64_t t = m_bp.select0(n - b - 1);
        excess_t exc_t = excess_t(t - 2 * (n - b - 1));
        assert(exc_t - 1 == m_bp.excess(t + 1));

        uint64_t x = m_bp.select0(n - b);
        uint64_t y = m_bp.select0(n - a);

        excess_t exc_w;
        uint64_t w = m_bp.excess_rmq(x, y, exc_w);
        uint64_t rank0_w = (w - uint64_t(exc_w)) / 2;
        assert(m_bp[w - 1] == 0);

        uint64_t ret;
        if (exc_w >= exc_t - 1) {
            ret = b;
        } else {
            ret = n - rank0_w;
        }

        assert(ret >= a);
        assert(ret <= b);
        return ret;
    }

    bp_vector const& get_bp() const {
        return m_bp;
    }

    uint64_t size() const {
        return m_bp.size() / 2 - 1;
    }

    size_t bytes() const {
        return m_bp.bytes();
    }

    template <typename Visitor>
    void visit(Visitor& visitor) {
        visitor.visit(m_bp);
    }

    void swap(cartesian_tree& other) {
        other.m_bp.swap(m_bp);
    }

protected:
    template <typename T>
    cartesian_tree(builder<T>* b) {
        bp_vector(&b->finalize(), false, true).swap(m_bp);
    }

    bp_vector m_bp;
};

}  // namespace autocomplete
