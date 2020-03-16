#pragma once

#include "compact_vector.hpp"
#include "util_types.hpp"

namespace autocomplete {

template <typename RMQ>
struct unsorted_list {
    static const uint32_t SCAN_THRESHOLD = 64;

    unsorted_list() {}

    void build(std::vector<id_type> const& list) {
        essentials::logger("building unsorted_list...");
        m_rmq.build(list, std::less<id_type>());
        m_list.build(list.begin(), list.size());
        essentials::logger("DONE");
    }

    uint32_t topk(const range r, const uint32_t k, std::vector<id_type>& topk,
                  bool unique = false  // return unique results
    ) {
        uint32_t range_len = r.end - r.begin;
        if (range_len <= k) {  // report everything in range
            for (uint32_t i = 0; i != range_len; ++i) {
                topk[i] = m_list.access(r.begin + i);
            }
            std::sort(topk.begin(), topk.begin() + range_len);
            return range_len;
        }

        scored_range sr;
        sr.r = {r.begin, r.end - 1};  // rmq needs inclusive ranges
        sr.min_pos = m_rmq.rmq(sr.r.begin, sr.r.end);
        sr.min_val = m_list.access(sr.min_pos);

        m_q.clear();
        m_q.push(sr);

        uint32_t i = 0;
        while (!m_q.empty()) {
            scored_range min = m_q.top();

            if (!unique or
                (unique and !std::binary_search(topk.begin(), topk.begin() + i,
                                                min.min_val))) {
                topk[i++] = min.min_val;
                if (i == k) break;
            }

            m_q.pop();

            if (min.min_pos > 0 and min.min_pos - 1 >= min.r.begin) {
                scored_range left;
                left.r = {min.r.begin, min.min_pos - 1};
                if (left.r.end - left.r.begin <= SCAN_THRESHOLD) {
                    left.min_pos = rmq(left.r.begin, left.r.end);
                } else {
                    left.min_pos = m_rmq.rmq(left.r.begin, left.r.end);
                }
                left.min_val = m_list.access(left.min_pos);
                m_q.push(left);
            }

            if (min.min_pos < size() - 1 and min.r.end >= min.min_pos + 1) {
                scored_range right;
                right.r = {min.min_pos + 1, min.r.end};
                if (right.r.end - right.r.begin <= SCAN_THRESHOLD) {
                    right.min_pos = rmq(right.r.begin, right.r.end);
                } else {
                    right.min_pos = m_rmq.rmq(right.r.begin, right.r.end);
                }
                right.min_val = m_list.access(right.min_pos);
                m_q.push(right);
            }
        }

        return i;
    }

    size_t size() const {
        return m_list.size();
    }

    size_t bytes() const {
        return m_rmq.bytes() + m_list.bytes();
    }

    template <typename Visitor>
    void visit(Visitor& visitor) {
        visitor.visit(m_rmq);
        visitor.visit(m_list);
    }

private:
    struct topk_queue {
        void push(scored_range sr) {
            m_q.push_back(sr);
            std::push_heap(m_q.begin(), m_q.end(), m_comparator);
        }

        scored_range top() {
            return m_q.front();
        }

        void pop() {
            std::pop_heap(m_q.begin(), m_q.end(), m_comparator);
            m_q.pop_back();
        }

        void clear() {
            m_q.clear();
        }

        bool empty() const {
            return m_q.empty();
        }

    private:
        std::vector<scored_range> m_q;

        typedef std::function<bool(scored_range const&, scored_range const&)>
            scrored_range_comparator_type;
        scrored_range_comparator_type m_comparator = [](scored_range const& l,
                                                        scored_range const& r) {
            return scored_range::greater(l, r);
        };
    };

    topk_queue m_q;
    RMQ m_rmq;
    compact_vector m_list;

    uint64_t rmq(uint64_t lo, uint64_t hi) {  // inclusive endpoints
        uint64_t pos = lo;
        id_type min = id_type(-1);
        for (uint64_t i = lo; i <= hi; ++i) {
            id_type val = m_list.access(i);
            if (val < min) {
                min = val;
                pos = i;
            }
        }
        return pos;
    }
};

}  // namespace autocomplete