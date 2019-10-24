#pragma once

#include "parameters.hpp"
#include "util_types.hpp"

namespace autocomplete {

template <typename Nodes, typename Pointers, typename LeftExtremes,
          typename Sizes>
struct completion_trie {
    struct builder {
        builder() {}

        builder(parameters const& params)
            : m_size(params.num_completions)
            , m_nodes(params.num_levels)
            , m_pointers(params.num_levels - 1)
            , m_left_extremes(params.num_levels)
            , m_sizes(params.num_levels) {
            essentials::logger("building completion_trie...");

            m_doc_ids.reserve(params.num_completions);
            uint32_t levels = params.num_levels;
            for (uint32_t i = 0; i != levels; ++i) {
                m_nodes[i].reserve(params.nodes_per_level[i]);
                m_left_extremes[i].reserve(params.nodes_per_level[i]);
                m_sizes[i].reserve(params.nodes_per_level[i]);
                if (i != levels - 1) {
                    m_pointers[i].reserve(params.nodes_per_level[i] + 1);
                    m_pointers[i].push_back(0);
                }
            }

            std::vector<uint32_t> offsets(levels - 1, 0);
            std::ifstream input(
                (params.collection_basename + ".mapped").c_str(),
                std::ios_base::in);
            completion_iterator it(params, input);
            completion_type prev;
            prev.push_back(global::terminator);
            range r{0, 0};

            while (input) {
                auto& record = *it;
                auto& curr = record.completion;
                m_doc_ids.push_back(record.doc_id);

                uint32_t prev_s = prev.size() - 1;
                uint32_t curr_s = curr.size() - 1;
                uint32_t l = 0;  // |lcp(curr,prev)|

                while (l != curr_s and l != prev_s and curr[l] == prev[l]) ++l;

                for (uint32_t i = l;
                     prev_s > 0 and i < levels - 1 and i <= prev_s; ++i) {
                    uint32_t last = m_pointers[i].back();
                    m_pointers[i].push_back(last + offsets[i]);
                    offsets[i] = 0;
                }

                uint32_t one = l > 0 ? 1 : 0;
                for (uint32_t i = l; i < curr_s + one; ++i) {
                    ++offsets[i - one];
                }

                for (uint32_t i = l; prev_s > 0 and i <= prev_s; ++i) {
                    m_sizes[i].push_back(r.end - 1);
                }

                for (uint32_t i = l; i <= curr_s; ++i) {
                    m_nodes[i].push_back(curr[i]);
                    m_left_extremes[i].push_back(r.begin);
                }

                prev.swap(curr);
                ++it;
                ++r.begin;
                ++r.end;
            }

            input.close();

            for (uint32_t i = 0; i != prev.size(); ++i) {
                m_sizes[i].push_back(r.end - 1);
            }

            for (uint32_t i = 0; i != levels - 1; ++i) {
                m_pointers[i].push_back(m_nodes[i + 1].size());
            }

            // NOTE: take the lengths of the ranges (minus 1)
            // and prefix sum them
            for (uint32_t i = 0; i != levels; ++i) {
                auto const& begin = m_left_extremes[i];
                auto& end = m_sizes[i];
                for (uint32_t k = 0, sum = 0; k != end.size(); ++k) {
                    end[k] -= begin[k];
                    end[k] += sum;
                    sum = end[k];
                }
            }

            // NOTE: subtract i from the i-th begin
            // because these are strictly increasing
            // (it works well with Elias-Fano because
            // we only need random access and not NextGEQ)
            for (auto& vec : m_left_extremes) {
                for (uint32_t i = 0; i != vec.size(); ++i) {
                    vec[i] -= i;
                }
            }

            essentials::logger("DONE");
        }

        void swap(builder& other) {
            uint32_t levels = m_nodes.size();
            for (uint32_t i = 0; i != levels; ++i) {
                other.m_left_extremes[i].swap(m_left_extremes[i]);
                other.m_sizes[i].swap(m_sizes[i]);
                other.m_nodes[i].swap(m_nodes[i]);
                if (i != levels - 1) {
                    other.m_pointers[i].swap(m_pointers[i]);
                }
            }
            other.m_doc_ids.swap(m_doc_ids);
        }

        void build(completion_trie<Nodes, Pointers, LeftExtremes, Sizes>& ct) {
            uint32_t levels = m_nodes.size();
            ct.m_size = m_size;
            ct.m_nodes.resize(levels);
            ct.m_pointers.resize(levels - 1);
            ct.m_left_extremes.resize(levels);
            ct.m_sizes.resize(levels);

            for (uint32_t i = 0; i != levels; ++i) {
                if (i != levels - 1) {
                    ct.m_pointers[i].build(m_pointers[i]);
                }
                ct.m_left_extremes[i].build(m_left_extremes[i]);
                ct.m_sizes[i].build(m_sizes[i]);
                if (i) {
                    ct.m_nodes[i].build(m_nodes[i], m_pointers[i - 1]);
                } else {
                    ct.m_nodes[i].build(m_nodes[i]);
                }
            }

            builder().swap(*this);
        }

        std::vector<id_type>& doc_ids() {
            return m_doc_ids;
        }

    private:
        uint32_t m_size;
        std::vector<std::vector<uint32_t>> m_nodes;
        std::vector<std::vector<uint32_t>> m_pointers;
        std::vector<std::vector<uint32_t>> m_left_extremes;
        std::vector<std::vector<uint32_t>> m_sizes;
        std::vector<id_type> m_doc_ids;
    };

    completion_trie() {}

    // If the last token of the query is not completely specified,
    // then we search for its lexicographic range among the children of prefix.
    // Return [a,b)
    range locate_prefix(completion_type const& prefix,
                        range suffix_lex_range) const {
        range r = global::invalid_range;
        range pointer{0, m_nodes.front().size()};
        uint32_t i = 0;

        for (; i < prefix.size(); ++i) {
            uint64_t pos = m_nodes[i].find(pointer, prefix[i]);
            if (pos == global::not_found) return global::invalid_range;
            pointer = m_pointers[i][pos];
        }

        if (i < m_nodes.size()) {
            range q = m_nodes[i].find(pointer, suffix_lex_range);
            assert(q.begin <= q.end);
            if (q.begin == q.end) return global::invalid_range;
            assert(q.end > q.begin);
            uint64_t begin = q.begin;
            uint64_t end = q.end - 1;
            r.begin = m_left_extremes[i].access(begin) + begin;
            r.end =
                end != begin ? m_left_extremes[i].access(end) + end : r.begin;
            uint64_t size = m_sizes[i].access(end) -
                            (end ? m_sizes[i].access(end - 1) : 0) + 1;
            r.end += size;
        }

        assert(r.is_valid());
        return r;
    }

    // NOTE: not used
    bool is_member(completion_type const& c) const {
        assert(c.size() > 0);
        range pointer{0, m_nodes.front().size()};
        uint32_t levels = c.size() - 1;
        for (uint32_t i = 0; i <= levels; ++i) {
            uint64_t pos = m_nodes[i].find(pointer, c[i]);
            if (pos == global::not_found) return false;
            if (i != levels) pointer = m_pointers[i][pos];
        }
        return true;
    }

    void print_stats() const;

    size_t bytes() const {
        size_t bytes = sizeof(m_size);
        bytes += sizeof(size_t) * 4;  // for std::vector's size()
        uint32_t levels = m_nodes.size();
        for (uint32_t i = 0; i != levels; ++i) {
            bytes += m_nodes[i].bytes();
            bytes += m_left_extremes[i].bytes();
            bytes += m_sizes[i].bytes();
            if (i != levels - 1) {
                bytes += m_pointers[i].bytes();
            }
        }
        return bytes;
    }

    size_t nodes_bytes() const {
        size_t bytes = sizeof(size_t);
        for (auto const& v : m_nodes) bytes += v.bytes();
        return bytes;
    }

    size_t pointers_bytes() const {
        size_t bytes = sizeof(size_t);
        for (uint32_t i = 0; i != m_nodes.size() - 1; ++i) {
            bytes += m_pointers[i].bytes();
        }
        return bytes;
    }

    size_t left_extremes_bytes() const {
        size_t bytes = sizeof(size_t);
        for (auto const& v : m_left_extremes) bytes += v.bytes();
        return bytes;
    }

    size_t sizes_bytes() const {
        size_t bytes = sizeof(size_t);
        for (auto const& v : m_sizes) bytes += v.bytes();
        return bytes;
    }

    uint32_t size() const {
        return m_size;
    }

    template <typename Visitor>
    void visit(Visitor& visitor) {
        visitor.visit(m_size);
        visitor.visit(m_nodes);
        visitor.visit(m_pointers);
        visitor.visit(m_left_extremes);
        visitor.visit(m_sizes);
    }

private:
    uint32_t m_size;
    std::vector<Nodes> m_nodes;
    std::vector<Pointers> m_pointers;
    std::vector<LeftExtremes> m_left_extremes;
    std::vector<Sizes> m_sizes;
};
}  // namespace autocomplete
