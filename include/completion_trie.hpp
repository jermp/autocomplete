#pragma once

#include "util_types.hpp"

namespace autocomplete {

template <typename Nodes, typename Pointers, typename LeftExtremes,
          typename Sizes>
struct completion_trie {
    struct builder {
        builder() {}

        builder(parameters const& params)
            : m_nodes(params.num_levels)
            , m_pointers(params.num_levels - 1)
            , m_left_extremes(params.num_levels)
            , m_sizes(params.num_levels) {
            for (uint32_t i = 0; i != params.num_levels; ++i) {
                m_nodes[i].reserve(params.nodes_per_level[i]);
                m_left_extremes[i].reserve(params.nodes_per_level[i]);
                m_sizes[i].reserve(params.nodes_per_level[i]);
                if (i != params.num_levels - 1) {
                    m_pointers[i].reserve(params.nodes_per_level[i]);
                    m_pointers[i].push_back(0);
                }
            }

            std::vector<uint32_t> offsets(params.num_levels - 1, 0);
            std::ifstream input(params.collection_basename, std::ios_base::in);
            completion_iterator it(params, input);
            completion prev = completion::empty();
            uint32_t begin = 0;
            uint32_t end = 0;

            while (input) {
                completion& curr = *it;

                // std::cout << "\n" << curr << std::endl;
                // std::cout << "prev size: " << prev.size() << std::endl;
                // std::cout << "curr size: " << curr.size() << std::endl;

                uint32_t l = 0;  // |lcp(curr,prev)|

                while (l != curr.size() and l != prev.size() and
                       curr[l] == prev[l]) {
                    ++l;
                }

                std::cout << "|lcp| = " << l << std::endl;

                for (uint32_t i = l;
                     prev.size() > 0 and i < params.num_levels - 1 and
                     i <= prev.size();
                     ++i) {
                    uint32_t last = m_pointers[i].back();
                    m_pointers[i].push_back(last + offsets[i]);
                    offsets[i] = 0;
                }

                if (l == 0) {  // special case
                    ++offsets[0];
                } else {
                    for (uint32_t i = l; i <= curr.size(); ++i) {
                        ++offsets[i - 1];
                    }
                }

                for (uint32_t i = l; prev.size() > 0 and i <= prev.size();
                     ++i) {
                    m_sizes[i].push_back(end - 1);
                }

                for (uint32_t i = l; i <= curr.size(); ++i) {
                    m_nodes[i].push_back(curr[i]);
                    m_left_extremes[i].push_back(begin);
                }

                // for (auto x : offsets) {
                //     std::cout << x << " ";
                // }
                // std::cout << std::endl;

                prev.swap(curr);
                ++it;
                ++begin;
                ++end;
            }

            input.close();

            for (uint32_t i = 0; i != prev.size() + 1; ++i) {
                m_sizes[i].push_back(end - 1);
            }

            // NOTE: take the lengths of the ranges (minus 1)
            // and prefix sum them
            for (uint32_t i = 0; i != params.num_levels; ++i) {
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

            std::cout << "nodes: " << std::endl;
            std::cout << "| ";
            for (auto const& vec : m_nodes) {
                for (auto x : vec) {
                    std::cout << x << " ";
                }
                std::cout << "| ";
            }
            std::cout << std::endl;

            std::cout << "pointers: " << std::endl;
            std::cout << "| ";
            for (auto const& vec : m_pointers) {
                for (auto x : vec) {
                    std::cout << x << " ";
                }
                std::cout << "| ";
            }
            std::cout << std::endl;

            std::cout << "left_extremes: " << std::endl;
            std::cout << "| ";
            for (auto const& vec : m_left_extremes) {
                for (auto x : vec) {
                    std::cout << x << " ";
                }
                std::cout << "| ";
            }
            std::cout << std::endl;

            std::cout << "sizes: " << std::endl;
            std::cout << "| ";
            for (auto const& vec : m_sizes) {
                for (auto x : vec) {
                    std::cout << x << " ";
                }
                std::cout << "| ";
            }
            std::cout << std::endl;
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
        }

        void build(completion_trie<Nodes, Pointers, LeftExtremes, Sizes>& ct) {
            uint32_t levels = m_nodes.size();
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

    private:
        std::vector<std::vector<uint32_t>> m_nodes;
        std::vector<std::vector<uint32_t>> m_pointers;
        std::vector<std::vector<uint32_t>> m_left_extremes;
        std::vector<std::vector<uint32_t>> m_sizes;
    };

    completion_trie() {}

    // void print_stats(essentials::json_lines& stats, size_t bytes);

    size_t bytes() const {
        size_t bytes = sizeof(size_t) * 4;  // for std::vector's size()
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

    void print() const {
        uint32_t levels = m_nodes.size();
        for (uint32_t i = 0; i != levels; ++i) {
            std::cout << "level-" << i << "\n";

            std::cout << "\t nodes: ";
            m_nodes[i].print();
            std::cout << std::endl;

            if (i != levels - 1) {
                std::cout << "\t pointers: ";
                m_pointers[i].print();
                std::cout << std::endl;
            }

            std::cout << "\t left_extremes: ";
            m_left_extremes[i].print();
            std::cout << std::endl;

            std::cout << "\t sizes: ";
            m_sizes[i].print();
            std::cout << std::endl;
        }
    }

    template <typename Visitor>
    void visit(Visitor& visitor) {
        visitor.visit(m_nodes);
        visitor.visit(m_pointers);
        visitor.visit(m_left_extremes);
        visitor.visit(m_sizes);
    }

private:
    std::vector<Nodes> m_nodes;
    std::vector<Pointers> m_pointers;
    std::vector<LeftExtremes> m_left_extremes;
    std::vector<Sizes> m_sizes;
};
}  // namespace autocomplete
