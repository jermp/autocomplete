#pragma once

#include "parameters.hpp"
#include "integer_codes.hpp"
#include "building_util.hpp"
#include "ef/ef_sequence.hpp"

namespace autocomplete {

template <typename ListType>
struct forward_index {
    typedef ListType forward_list_type;
    typedef typename forward_list_type::iterator forward_list_iterator_type;
    typedef uncompressed_list<uint8_t> permutation_list_type;
    typedef
        typename permutation_list_type::iterator permutation_list_iterator_type;

    struct builder {
        builder() {}

        builder(parameters const& params)
            : m_num_integers(0)
            , m_num_terms(params.num_terms) {
            essentials::logger("building forward_index...");

            uint64_t num_completions = params.num_completions;

            std::ifstream input(
                (params.collection_basename + ".forward").c_str(),
                std::ios_base::in);

            std::vector<id_type> list;
            std::vector<id_type> sorted_permutation;
            std::vector<uint8_t> permutation;

            m_pointers.push_back(0);

            for (uint64_t i = 0; i != num_completions; ++i) {
                list.clear();
                sorted_permutation.clear();
                permutation.clear();

                uint32_t n = 0;
                input >> n;
                assert(n > 0 and n < constants::MAX_NUM_TERMS_PER_QUERY);
                m_num_integers += n;
                list.reserve(n);
                sorted_permutation.reserve(n);

                for (uint64_t k = 0; k != n; ++k) {
                    id_type x;
                    input >> x;
                    list.push_back(x);
                    sorted_permutation.push_back(k);
                }

                write_gamma_nonzero(m_bvb, n);
                if (ListType::is_byte_aligned) util::push_pad(m_bvb);

                std::sort(
                    sorted_permutation.begin(), sorted_permutation.end(),
                    [&](id_type l, id_type r) { return list[l] < list[r]; });

                permutation.resize(n);
                for (uint32_t i = 0; i != n; ++i) {
                    permutation[sorted_permutation[i]] = i;
                }

                std::sort(list.begin(), list.end());
                forward_list_type::build(m_bvb, list.begin(), m_num_terms + 1,
                                         n);
                util::push_pad(m_bvb);
                m_pointers.push_back(m_bvb.size());

                permutation_list_type::build(m_bvb, permutation.begin(), n + 1,
                                             n);
                m_pointers.push_back(m_bvb.size());
            }

            m_pointers.pop_back();
            input.close();
            essentials::logger("DONE");
        }

        void swap(forward_index::builder& other) {
            std::swap(other.m_num_integers, m_num_integers);
            std::swap(other.m_num_terms, m_num_terms);
            other.m_pointers.swap(m_pointers);
            other.m_bvb.swap(m_bvb);
        }

        void build(forward_index<ListType>& fi) {
            fi.m_num_integers = m_num_integers;
            fi.m_num_terms = m_num_terms;
            fi.m_pointers.build(m_pointers);
            fi.m_data.build(&m_bvb);
            builder().swap(*this);
        }

    private:
        uint64_t m_num_integers;
        uint64_t m_num_terms;
        std::vector<uint64_t> m_pointers;
        bit_vector_builder m_bvb;
    };

    forward_index() {}

    bool intersects(id_type doc_id, range r) {
        return get(doc_id).intersects(r);
    }

    struct permuting_iterator_type {
        permuting_iterator_type(forward_list_iterator_type const& sorted,
                                permutation_list_iterator_type const& permuted)
            : m_i(0)
            , m_sorted(sorted)
            , m_permuted(permuted) {
            assert(sorted.size() == permuted.size());
        }

        uint32_t size() const {
            return m_sorted.size();
        }

        id_type operator*() {
            return m_sorted.access(m_permuted.access(m_i));
        }

        void operator++() {
            ++m_i;
        }

    private:
        uint32_t m_i;
        forward_list_iterator_type m_sorted;
        permutation_list_iterator_type m_permuted;
    };

    permuting_iterator_type iterator(id_type doc_id) {
        uint64_t offset = m_pointers.access(doc_id * 2);
        bits_iterator<bit_vector> it(m_data, offset);
        uint64_t n = read_gamma_nonzero(it);
        if (ListType::is_byte_aligned) util::eat_pad(it);
        forward_list_iterator_type it_sorted(m_data, it.position(),
                                             m_num_terms + 1, n);
        offset = m_pointers.access(doc_id * 2 + 1);
        permutation_list_iterator_type it_permutation(m_data, offset, n + 1, n);
        return permuting_iterator_type(it_sorted, it_permutation);
    }

    uint64_t num_integers() const {
        return m_num_integers;
    }

    uint64_t num_terms() const {
        return m_num_terms;
    }

    uint64_t num_docs() const {
        return m_pointers.size();
    }

    size_t data_bytes() const {
        return m_data.bytes();
    }

    size_t pointer_bytes() const {
        return m_pointers.bytes();
    }

    size_t bytes() const {
        return essentials::pod_bytes(m_num_integers) +
               essentials::pod_bytes(m_num_terms) + m_pointers.bytes() +
               m_data.bytes();
    }

    template <typename Visitor>
    void visit(Visitor& visitor) {
        visitor.visit(m_num_integers);
        visitor.visit(m_num_terms);
        visitor.visit(m_pointers);
        visitor.visit(m_data);
    }

private:
    uint64_t m_num_integers;
    uint64_t m_num_terms;
    ef::ef_sequence m_pointers;
    bit_vector m_data;

    forward_list_iterator_type get(id_type doc_id) {
        uint64_t offset = m_pointers.access(doc_id * 2);
        bits_iterator<bit_vector> it(m_data, offset);
        uint64_t n = read_gamma_nonzero(it);
        if (ListType::is_byte_aligned) util::eat_pad(it);
        return {m_data, it.position(), m_num_terms + 1, n};
    }
};

}  // namespace autocomplete