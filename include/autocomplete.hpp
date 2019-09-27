#pragma once

#include "util_types.hpp"
#include "strings_pool.hpp"

namespace autocomplete {

template <typename CompletionTrie, typename UnsortedDocsList,
          typename Dictionary, typename InvertedIndex, typename ForwardIndex>
struct autocomplete {
    static const size_t POOL_SIZE = 1024;

    autocomplete() {
        m_pool.resize(POOL_SIZE);
    }

    autocomplete(parameters const& params) {
        typename CompletionTrie::builder ct_builder(params);
        typename Dictionary::builder di_builder(params);
        typename InvertedIndex::builder ii_builder(params);
        typename ForwardIndex::builder fi_builder(params);
        m_unsorted_docs_list.build(ct_builder.doc_ids());
        ct_builder.build(m_completion_trie);
        di_builder.build(m_dictionary);
        ii_builder.build(m_inverted_index);
        fi_builder.build(m_forward_index);
        m_pool.resize(POOL_SIZE);
    }

    strings_pool::iterator prefix_topk(std::string& query, uint32_t k) {
        m_pool.clear();
        m_pool.init();
        uint32_t num_terms = parse(query);
        completion prefix(num_terms);
        forward_byte_range_iterator it(string_to_byte_range(query));

        for (uint32_t i = 0; i != num_terms; ++i) {
            byte_range br = it.next();
            id_type term_id = m_dictionary.locate(br);
            prefix.push_back(term_id);
        }

        // std::cout << "prefix " << prefix << std::endl;

        range r = m_completion_trie.prefix_range(prefix);

        std::vector<id_type> topk(k);
        uint32_t num_completions = m_unsorted_docs_list.topk(r, k, topk);

        for (uint32_t i = 0; i != num_completions; ++i) {
            id_type doc_id = topk[i];
            auto it = m_forward_index[doc_id];
            uint64_t offset = m_pool.bytes();
            uint8_t* decoded = m_pool.data() + offset;
            for (uint32_t j = 0; j != it.size(); ++j) {
                id_type term_id = it.access(j);
                uint8_t len = m_dictionary.extract(term_id, decoded);
                decoded += len;
                offset += len;
                if (j != it.size() - 1) {
                    *decoded++ = ' ';
                    ++offset;
                }
            }
            m_pool.push_back_offset(offset);
        }

        return m_pool.begin();
    }

    size_t bytes() const {
        return m_completion_trie.bytes() + m_unsorted_docs_list.bytes() +
               m_dictionary.bytes() + m_inverted_index.bytes() +
               m_forward_index.bytes();
    }

    template <typename Visitor>
    void visit(Visitor& visitor) {
        visitor.visit(m_completion_trie);
        visitor.visit(m_unsorted_docs_list);
        visitor.visit(m_dictionary);
        visitor.visit(m_inverted_index);
        visitor.visit(m_forward_index);
    }

private:
    CompletionTrie m_completion_trie;
    UnsortedDocsList m_unsorted_docs_list;
    Dictionary m_dictionary;
    InvertedIndex m_inverted_index;
    ForwardIndex m_forward_index;
    strings_pool m_pool;
};
}  // namespace autocomplete