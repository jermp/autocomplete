#pragma once

#include <algorithm>
#include <unordered_map>

#include "bit_vector.hpp"
#include "ef/ef_sequence.hpp"
#include "ef/compact_ef.hpp"
#include "parameters.hpp"

namespace autocomplete {

template <typename InvertedListType>
struct blocked_inverted_index {
    typedef typename InvertedListType::iterator docs_iterator_type;
    typedef typename ef::compact_ef::iterator offsets_iterator_type;
    typedef bits_getter terms_iterator_type;

    struct builder {
        builder() {}

        builder(parameters const& params, float c)
            : m_num_integers(0)
            , m_num_docs(params.universe)
            , m_num_terms(params.num_terms) {
            if (!(c > 0.0 and c <= 1.0)) {
                throw std::runtime_error("c must be in (0,1]");
            }

            essentials::logger("building blocked_inverted_index with c = " +
                               std::to_string(c) + "...");

            uint64_t num_terms = params.num_terms;
            m_minimal_doc_ids.reserve(num_terms);

            uint64_t m = m_num_docs * c;

            std::ifstream input(
                (params.collection_basename + ".inverted").c_str(),
                std::ios_base::in);

            m_pointers_to_lists.push_back(0);
            m_pointers_to_offsets.push_back(0);
            m_pointers_to_terms.push_back(0);

            std::unordered_map<id_type, std::vector<id_type>> terms;
            terms.reserve(m_num_docs);
            for (uint64_t doc_id = 0; doc_id != m_num_docs; ++doc_id) {
                terms[doc_id] = std::vector<id_type>();
            }

            std::vector<id_type> union_of_lists;
            union_of_lists.reserve(m);
            uint64_t num_postings_in_block = 0;

            id_type lower_bound = 1;
            for (uint64_t term_id = 1; term_id <= num_terms; ++term_id) {
                uint32_t n = 0;
                input >> n;
                num_postings_in_block += n;
                m_num_integers += n;

                for (uint64_t k = 0; k != n; ++k) {
                    id_type doc_id;
                    input >> doc_id;
                    union_of_lists.push_back(doc_id);
                    assert(term_id >= lower_bound);
                    terms[doc_id].push_back(term_id - lower_bound);
                    if (k == 0) {
                        m_minimal_doc_ids.push_back(doc_id);
                    }
                }

                if (num_postings_in_block >= m or term_id == num_terms) {
                    lower_bound = term_id;
                    m_blocks.push_back(term_id);
                    std::sort(union_of_lists.begin(), union_of_lists.end());
                    auto end = std::unique(union_of_lists.begin(),
                                           union_of_lists.end());
                    uint64_t size = std::distance(union_of_lists.begin(), end);
                    // std::cout << "union has size " << size << std::endl;

                    m_lists.append_bits(size, 32);
                    InvertedListType::build(m_lists, union_of_lists.begin(),
                                            m_num_docs, size);
                    m_pointers_to_lists.push_back(m_lists.size());

                    std::vector<id_type> term_list;
                    std::vector<uint64_t> offset_list;

                    uint64_t offset = 0;
                    for (uint64_t i = 0; i != size; ++i) {
                        auto doc_id = union_of_lists[i];
                        auto& set_of_term_ids = terms[doc_id];
                        assert(std::is_sorted(set_of_term_ids.begin(),
                                              set_of_term_ids.end()));
                        offset_list.push_back(offset);

                        uint64_t n = set_of_term_ids.size();
                        term_list.reserve(n);
                        for (auto t : set_of_term_ids) {
                            term_list.push_back(t);
                        }

                        offset += n;
                        set_of_term_ids.clear();
                    }

                    offset_list.push_back(offset);

                    m_offsets.append_bits(offset_list.size(), 32);
                    m_offsets.append_bits(offset_list.back() + 1, 32);
                    ef::compact_ef::build(m_offsets, offset_list.begin(),
                                          offset_list.back() + 1,
                                          offset_list.size());
                    m_pointers_to_offsets.push_back(m_offsets.size());

                    auto max =
                        *std::max_element(term_list.begin(), term_list.end());
                    uint64_t width = util::ceil_log2(max + 1);
                    if (width == 0) width = 1;
                    // std::cout << "using " << width << " [bpi]" << std::endl;
                    m_terms.append_bits(width, 6);
                    for (auto t : term_list) m_terms.append_bits(t, width);
                    m_pointers_to_terms.push_back(m_terms.size());

                    num_postings_in_block = 0;
                    union_of_lists.clear();
                }
            }

            input.close();

            m_pointers_to_lists.pop_back();
            m_pointers_to_offsets.pop_back();
            m_pointers_to_terms.pop_back();

            essentials::logger("DONE");
        }

        std::vector<id_type>& minimal_doc_ids() {
            return m_minimal_doc_ids;
        }

        void swap(blocked_inverted_index::builder& other) {
            std::swap(other.m_num_integers, m_num_integers);
            std::swap(other.m_num_docs, m_num_docs);
            std::swap(other.m_num_terms, m_num_terms);

            other.m_blocks.swap(m_blocks);

            other.m_pointers_to_lists.swap(m_pointers_to_lists);
            other.m_pointers_to_offsets.swap(m_pointers_to_offsets);
            other.m_pointers_to_terms.swap(m_pointers_to_terms);

            other.m_lists.swap(m_lists);
            other.m_offsets.swap(m_offsets);
            other.m_terms.swap(m_terms);

            other.m_minimal_doc_ids.swap(m_minimal_doc_ids);
        }

        void build(blocked_inverted_index<InvertedListType>& ii) {
            ii.m_num_integers = m_num_integers;
            ii.m_num_docs = m_num_docs;
            ii.m_num_terms = m_num_terms;

            ii.m_blocks.swap(m_blocks);

            ii.m_pointers_to_lists.build(m_pointers_to_lists);
            ii.m_pointers_to_offsets.build(m_pointers_to_offsets);
            ii.m_pointers_to_terms.build(m_pointers_to_terms);

            ii.m_lists.build(&m_lists);
            ii.m_offsets.build(&m_offsets);
            ii.m_terms.build(&m_terms);

            builder().swap(*this);
        }

    private:
        uint64_t m_num_integers;
        uint64_t m_num_docs;
        uint64_t m_num_terms;

        std::vector<uint32_t> m_blocks;

        std::vector<uint64_t> m_pointers_to_lists;
        std::vector<uint64_t> m_pointers_to_offsets;
        std::vector<uint64_t> m_pointers_to_terms;

        bit_vector_builder m_lists;
        bit_vector_builder m_offsets;
        bit_vector_builder m_terms;

        std::vector<id_type> m_minimal_doc_ids;
    };

    blocked_inverted_index() {}

    uint64_t num_integers() const {
        return m_num_integers;
    }

    uint64_t num_terms() const {
        return m_num_terms;
    }

    uint64_t num_docs() const {
        return m_num_docs;
    }

    uint64_t num_blocks() const {
        return m_blocks.size();
    }

    size_t bytes() const {
        return essentials::pod_bytes(m_num_integers) +
               essentials::pod_bytes(m_num_docs) +
               essentials::pod_bytes(m_num_terms) +
               essentials::vec_bytes(m_blocks) + m_pointers_to_lists.bytes() +
               m_lists.bytes() + m_pointers_to_offsets.bytes() +
               m_offsets.bytes() + m_pointers_to_terms.bytes() +
               m_terms.bytes();
    }

    size_t blocks_bytes() const {
        return essentials::vec_bytes(m_blocks);
    }

    size_t pointers_bytes() const {
        return m_pointers_to_lists.bytes() + m_pointers_to_offsets.bytes() +
               m_pointers_to_terms.bytes();
    }

    size_t docs_bytes() const {
        return m_lists.bytes();
    }

    size_t offsets_bytes() const {
        return m_offsets.bytes();
    }

    size_t terms_bytes() const {
        return m_terms.bytes();
    }

    uint32_t block_id(id_type term_id) const {
        auto it = std::upper_bound(m_blocks.begin(), m_blocks.end(), term_id);
        uint32_t id = std::distance(m_blocks.begin(), it);
        if (id > 0 and *std::prev(it) == term_id) return id - 1;
        return id;
    }

    uint32_t block_boundary(uint32_t block_id) const {
        assert(block_id < m_blocks.size());
        return m_blocks[block_id];
    }

    struct block_type {
        docs_iterator_type docs_iterator;
        offsets_iterator_type offsets_iterator;
        terms_iterator_type terms_iterator;
        std::vector<id_type> term_ids;
        id_type lower_bound;
    };

    struct intersection_iterator_type {
        intersection_iterator_type(blocked_inverted_index const* ii,
                                   std::vector<id_type>& term_ids,
                                   const range r)
            : m_i(0)
            , m_num_docs(ii->num_docs())
            , m_suffix(r) {
            assert(r.is_valid());
            assert(!term_ids.empty());
            assert(std::is_sorted(term_ids.begin(), term_ids.end()));
            assert(std::unique(term_ids.begin(), term_ids.end()) ==
                   term_ids.end());

            m_blocks.reserve(term_ids.size());  // at most
            uint32_t current_block_id = ii->block_id(term_ids.front());
            uint32_t i = 0;
            uint32_t prev_i = 0;
            for (; i != term_ids.size(); ++i) {
                auto term_id = term_ids[i];
                assert(term_id > 0);
                uint32_t b = ii->block_id(term_id);
                if (b > current_block_id) {
                    auto block = ii->block(current_block_id);
                    block.term_ids.reserve(term_ids.size());  // at most
                    for (; prev_i != i; ++prev_i) {
                        block.term_ids.push_back(term_ids[prev_i]);
                    }
                    m_blocks.push_back(std::move(block));
                }
                current_block_id = b;
            }

            auto block = ii->block(current_block_id);
            block.term_ids.reserve(term_ids.size());  // at most
            for (; prev_i != i; ++prev_i) {
                block.term_ids.push_back(term_ids[prev_i]);
            }
            m_blocks.push_back(std::move(block));

            std::sort(m_blocks.begin(), m_blocks.end(),
                      [](auto const& l, auto const& r) {
                          return l.docs_iterator.size() <
                                 r.docs_iterator.size();
                      });

            m_candidate = m_blocks[0].docs_iterator.access(0);

            next();
        }

        bool has_next() const {
            return m_candidate < m_num_docs;
        }

        id_type operator*() {
            return m_candidate;
        }

        void operator++() {
            assert(m_i == m_blocks.size());
            if (m_blocks.size() > 1) {
                m_candidate = m_blocks[0].docs_iterator.next();
            }
            m_i = 0;
            next();
        }

    private:
        id_type m_candidate;
        size_t m_i;
        uint64_t m_num_docs;
        std::vector<block_type> m_blocks;
        std::vector<block_type> m_range;
        range m_suffix;

        bool in() {  // is candidate doc in intersection?

            auto& b = m_blocks[m_i];
            uint64_t pos = b.docs_iterator.position();
            if (pos == b.docs_iterator.size()) return false;
            uint64_t begin = b.offsets_iterator.access(pos);
            uint64_t end = b.offsets_iterator.access(pos + 1);
            assert(end > begin);
            if (end - begin < b.term_ids.size()) return false;

            uint64_t i = begin;
            for (auto x : b.term_ids) {
                bool found = false;
                for (; i != end; ++i) {
                    auto t = b.terms_iterator.access(i) + b.lower_bound;
                    if (t == x) {
                        found = true;
                        break;
                    }
                }
                if (!found) return false;
            }

            return true;
        }

        void next() {
            if (m_blocks.size() == 1) {
                while (m_candidate < m_num_docs and m_i != m_blocks.size()) {
                    assert(m_i == 0);
                    m_candidate = m_blocks[m_i].docs_iterator.next();
                    if (in()) ++m_i;
                }
            } else {
                while (m_candidate < m_num_docs and m_i != m_blocks.size()) {
                    // NOTE: since we work with unions of posting lists,
                    // next_geq by scan runs faster
                    auto val = m_blocks[m_i].docs_iterator.next_geq_by_scan(
                        m_candidate);
                    bool is_in = in();
                    if (val == m_candidate and is_in) {
                        ++m_i;
                    } else {
                        m_candidate = val + !is_in;
                        m_i = 0;
                    }
                }
            }
        }
    };

    intersection_iterator_type intersection_iterator(
        std::vector<id_type>& term_ids, const range r) {
        return intersection_iterator_type(this, term_ids, r);
    }

    block_type block(uint32_t block_id) const {
        assert(block_id < num_blocks());
        block_type b;
        b.term_ids.resize(0);
        b.lower_bound = block_id > 0 ? m_blocks[block_id - 1] : 1;

        {
            uint64_t offset = m_pointers_to_lists.access(block_id);
            uint32_t n = m_lists.get_bits(offset, 32);
            docs_iterator_type it(m_lists, offset + 32, m_num_docs, n);
            b.docs_iterator = it;
        }
        {
            uint64_t offset = m_pointers_to_offsets.access(block_id);
            uint32_t n = m_offsets.get_bits(offset, 32);
            uint32_t universe = m_offsets.get_bits(offset + 32, 32);
            offsets_iterator_type it(m_offsets, offset + 32 + 32, universe, n);
            b.offsets_iterator = it;
        }
        {
            uint64_t offset = m_pointers_to_terms.access(block_id);
            uint32_t width = m_terms.get_bits(offset, 6);
            terms_iterator_type it(m_terms, offset + 6, width);
            b.terms_iterator = it;
        }

        return b;
    }

    template <typename Visitor>
    void visit(Visitor& visitor) {
        visitor.visit(m_num_integers);
        visitor.visit(m_num_docs);
        visitor.visit(m_num_terms);
        visitor.visit(m_blocks);
        visitor.visit(m_pointers_to_lists);
        visitor.visit(m_lists);
        visitor.visit(m_pointers_to_offsets);
        visitor.visit(m_offsets);
        visitor.visit(m_pointers_to_terms);
        visitor.visit(m_terms);
    }

private:
    uint64_t m_num_integers;
    uint64_t m_num_docs;
    uint64_t m_num_terms;

    std::vector<uint32_t> m_blocks;

    ef::ef_sequence m_pointers_to_lists;
    bit_vector m_lists;
    ef::ef_sequence m_pointers_to_offsets;
    bit_vector m_offsets;
    ef::ef_sequence m_pointers_to_terms;
    bit_vector m_terms;
};

}  // namespace autocomplete