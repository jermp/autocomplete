#pragma once

#include "util_types.hpp"
#include "min_heap.hpp"
#include "unsorted_list.hpp"
#include "minimal_docids.hpp"
#include "succinct_rmq/cartesian_tree.hpp"

namespace autocomplete {

typedef unsorted_list<cartesian_tree> unsorted_list_type;

template <typename Dictionary>
bool parse(Dictionary const& dict, std::string const& query,
           completion_type& prefix, byte_range& suffix, bool must_find_prefix) {
    byte_range_iterator it(string_to_byte_range(query));
    while (true) {
        suffix = it.next();
        if (!it.has_next()) break;
        auto term_id = dict.locate(suffix);
        if (term_id != global::invalid_term_id) {
            prefix.push_back(term_id);
        } else {
            if (must_find_prefix) return false;
        }
    }
    return true;
}

void deduplicate(completion_type& c) {
    std::sort(c.begin(), c.end());
    auto end = std::unique(c.begin(), c.end());
    c.resize(std::distance(c.begin(), end));
}

template <typename InvertedIndex>
uint32_t heap_topk(InvertedIndex const& index, const range r, const uint32_t k,
                   std::vector<id_type>& topk_scores) {
    assert(r.is_valid());

    typedef min_heap<typename InvertedIndex::iterator_type,
                     iterator_comparator<typename InvertedIndex::iterator_type>>
        min_priority_queue_type;

    min_priority_queue_type q;
    q.reserve(r.end - r.begin + 1);  // inclusive range
    assert(r.begin > 0);
    for (uint64_t term_id = r.begin; term_id <= r.end; ++term_id) {
        q.push_back(index.iterator(term_id - 1));
    }
    q.make_heap();

    uint32_t results = 0;

    while (!q.empty()) {
        auto& z = q.top();
        auto doc_id = *z;
        bool alread_present = std::binary_search(
            topk_scores.begin(), topk_scores.begin() + results, doc_id);
        if (!alread_present) {
            topk_scores[results++] = doc_id;
            if (results == k) return results;
        }
        z.next();
        if (!z.has_next()) q.pop();
        q.heapify();
    }

    return results;
}

}  // namespace autocomplete