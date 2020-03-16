#pragma once

#include "compact_vector.hpp"
#include "util_types.hpp"

namespace autocomplete {

template <typename RMQ, typename InvertedIndex>
struct minimal_docids {
    static const uint32_t SCAN_THRESHOLD = 64;
    typedef scored_range_with_list_iterator<
        typename InvertedIndex::iterator_type>
        range_type;
    typedef scored_range_with_list_iterator_comparator<
        typename range_type::iterator_type>
        comparator_range_type;

    minimal_docids() {}

    void build(std::vector<id_type> const& list) {
        essentials::logger("building minimal_docids...");
        m_rmq.build(list, std::less<id_type>());
        m_list.build(list.begin(), list.size());
        essentials::logger("DONE");
    }

    uint32_t topk(InvertedIndex const& index, const range r, const uint32_t k,
                  std::vector<id_type>& topk_scores) {
        range_type sr;
        sr.r = {r.begin, r.end - 1};  // rmq needs inclusive ranges
        sr.min_pos = m_rmq.rmq(sr.r.begin, sr.r.end);
        sr.min_val = m_list.access(sr.min_pos);

        m_q.clear();
        m_q.push(sr);

        uint32_t results = 0;
        while (!m_q.empty()) {
            auto& min = m_q.top();
            auto docid = min.minimum();
            bool alread_present = std::binary_search(
                topk_scores.begin(), topk_scores.begin() + results, docid);
            if (!alread_present) {
                topk_scores[results++] = docid;
                if (results == k) break;
            }

            if (min.is_open()) {
                min.iterator.next();
                if (!min.iterator.has_next()) {
                    m_q.pop();
                }
                m_q.heapify();
            } else {
                // save
                auto min_range = min.r;
                auto min_pos = min.min_pos;

                min.set_iterator(index);
                min.iterator.next();
                if (!min.iterator.has_next()) {
                    m_q.pop();
                }

                m_q.heapify();

                if (min_pos > 0 and min_pos - 1 >= min_range.begin) {
                    range_type left;
                    left.r = {min_range.begin, min_pos - 1};
                    if (left.r.end - left.r.begin <= SCAN_THRESHOLD) {
                        left.min_pos = rmq(left.r.begin, left.r.end);
                    } else {
                        left.min_pos = m_rmq.rmq(left.r.begin, left.r.end);
                    }
                    left.min_val = m_list.access(left.min_pos);
                    m_q.push(left);
                }

                if (min_pos < size() - 1 and min_range.end >= min_pos + 1) {
                    range_type right;
                    right.r = {min_pos + 1, min_range.end};
                    if (right.r.end - right.r.begin <= SCAN_THRESHOLD) {
                        right.min_pos = rmq(right.r.begin, right.r.end);
                    } else {
                        right.min_pos = m_rmq.rmq(right.r.begin, right.r.end);
                    }
                    right.min_val = m_list.access(right.min_pos);
                    m_q.push(right);
                }
            }
        }

        return results;
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
    typedef min_heap<range_type, comparator_range_type> min_priority_queue_type;
    min_priority_queue_type m_q;

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