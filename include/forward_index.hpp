#pragma once

#include "parameters.hpp"

namespace autocomplete {

template <typename ForwardList, typename Pointers>
struct forward_index {
    typedef typename ForwardList::iterator iterator_type;

    struct builder {
        builder() {}

        builder(parameters const& params)
            : m_num_terms(params.num_terms) {
            essentials::logger("building forward_index...");

            uint64_t num_completions = params.num_completions;

            std::ifstream input(
                (params.collection_basename + ".forward").c_str(),
                std::ios_base::in);

            std::vector<id_type> list;
            std::vector<id_type> sorted_permutation;
            std::vector<id_type> permutation;

            m_pointers.push_back(0);

            for (uint64_t i = 0; i != num_completions; ++i) {
                list.clear();
                sorted_permutation.clear();
                permutation.clear();

                uint32_t n = 0;
                input >> n;
                assert(n > 0);
                list.reserve(n);
                sorted_permutation.reserve(n);

                for (uint64_t k = 0; k != n; ++k) {
                    id_type x;
                    input >> x;
                    list.push_back(x);
                    sorted_permutation.push_back(k);
                }

                m_bvb.append_bits(n, 32);

                std::sort(
                    sorted_permutation.begin(), sorted_permutation.end(),
                    [&](id_type l, id_type r) { return list[l] < list[r]; });

                permutation.resize(n);
                for (uint32_t i = 0; i != n; ++i) {
                    permutation[sorted_permutation[i]] = i;
                }

                std::sort(list.begin(), list.end());
                ForwardList::build(m_bvb, list.begin(), n);
                m_pointers.push_back(m_bvb.size());

                // NOTE: permutation could be safely stored as
                // a 8-bit integer vector
                ForwardList::build(m_bvb, permutation.begin(), n);
                m_pointers.push_back(m_bvb.size());
            }

            m_pointers.pop_back();
            input.close();
            essentials::logger("DONE");
        }

        void swap(forward_index::builder& other) {
            std::swap(other.m_num_terms, m_num_terms);
            other.m_pointers.swap(m_pointers);
            other.m_bvb.swap(m_bvb);
        }

        void build(forward_index<ForwardList, Pointers>& fi) {
            fi.m_num_terms = m_num_terms;
            fi.m_pointers.build(m_pointers);
            fi.m_data.build(&m_bvb);
            builder().swap(*this);
        }

    private:
        uint64_t m_num_terms;
        std::vector<uint64_t> m_pointers;
        bit_vector_builder m_bvb;
    };

    forward_index() {}

    iterator_type iterator(id_type doc_id) {
        uint64_t offset = m_pointers.access(doc_id * 2);
        uint32_t n = m_data.get_bits(offset, 32);
        iterator_type it(m_data, offset + 32, m_num_terms, n, m_params);
        return it;
    }

    struct permuting_iterator_type {
        permuting_iterator_type(iterator_type sorted, iterator_type permutation)
            : m_i(0)
            , m_sorted(sorted)
            , m_permutation(permutation) {
            assert(sorted.size() == permutation.size());
        }

        uint32_t size() const {
            return m_sorted.size();
        }

        id_type operator*() {
            return m_sorted.access(m_permutation.access(m_i));
        }

        void operator++() {
            ++m_i;
        }

    private:
        uint32_t m_i;
        iterator_type m_sorted;
        iterator_type m_permutation;
    };

    permuting_iterator_type permuting_iterator(id_type doc_id) {
        uint64_t offset = m_pointers.access(doc_id * 2);
        uint32_t n = m_data.get_bits(offset, 32);
        iterator_type it_sorted_set(m_data, offset + 32, m_num_terms, n,
                                    m_params);
        offset = m_pointers.access(doc_id * 2 + 1);
        iterator_type it_permutation(m_data, offset, m_num_terms, n, m_params);
        return permuting_iterator_type(it_sorted_set, it_permutation);
    }

    uint64_t num_terms() const {
        return m_num_terms;
    }

    uint64_t num_docs() const {
        return m_pointers.size();
    }

    size_t bytes() const {
        return essentials::pod_bytes(m_num_terms) + m_pointers.bytes() +
               m_data.bytes();
    }

    template <typename Visitor>
    void visit(Visitor& visitor) {
        visitor.visit(m_num_terms);
        visitor.visit(m_pointers);
        visitor.visit(m_data);
    }

private:
    uint64_t m_num_terms;
    compression_parameters m_params;
    Pointers m_pointers;
    bit_vector m_data;
};

}  // namespace autocomplete