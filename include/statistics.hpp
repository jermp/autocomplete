#pragma once

#include "completion_trie.hpp"
#include "autocomplete.hpp"
#include "autocomplete2.hpp"
#include "autocomplete3.hpp"
#include "autocomplete4.hpp"

namespace autocomplete {

void print(std::string const& what, size_t bytes, size_t total_bytes,
           uint64_t num_completions) {
    std::cout << "  " << what << ": "
              << essentials::convert(bytes, essentials::MiB)
              << " [MiB]: " << static_cast<double>(bytes) / num_completions
              << " [bytes per completion] ";
    std::cout << "(" << (bytes * 100.0) / total_bytes << "%)" << std::endl;
}

void print_bpi(std::string const& what, size_t bytes, size_t integers) {
    std::cout << '\t' << what << ": " << (bytes * 8.0) / integers << " [bpi]"
              << std::endl;
}

void print_bps(std::string const& what, size_t bytes, size_t strings) {
    std::cout << '\t' << what << ": " << (bytes * 8.0) / strings << " [bps]"
              << std::endl;
}

template <typename Nodes, typename Pointers, typename LeftExtremes,
          typename Sizes>
void completion_trie<Nodes, Pointers, LeftExtremes, Sizes>::print_stats()
    const {
    size_t total_bytes = bytes();
    std::cout << "using " << essentials::convert(total_bytes, essentials::MiB)
              << " [MiB]" << std::endl;
    print_bps("nodes", nodes_bytes(), size());
    print_bps("pointers", pointers_bytes(), size());
    print_bps("left extremes", left_extremes_bytes(), size());
    print_bps("sizes", sizes_bytes(), size());
}

template <typename Completions, typename Dictionary, typename InvertedIndex,
          typename ForwardIndex>
void autocomplete<Completions, Dictionary, InvertedIndex,
                  ForwardIndex>::print_stats() const {
    size_t total_bytes = bytes();
    std::cout << "using " << essentials::convert(total_bytes, essentials::MiB)
              << " [MiB]: "
              << static_cast<double>(total_bytes) / m_completions.size()
              << " [bytes per completion] " << std::endl;

    print("completions", m_completions.bytes(), total_bytes,
          m_completions.size());
    print_bps("nodes", m_completions.nodes_bytes(), m_completions.size());
    print_bps("pointers", m_completions.pointers_bytes(), m_completions.size());
    print_bps("left extremes", m_completions.left_extremes_bytes(),
              m_completions.size());
    print_bps("sizes", m_completions.sizes_bytes(), m_completions.size());

    print("unsorted docs list", m_unsorted_docs_list.bytes(), total_bytes,
          m_completions.size());
    print("unsorted minimal docs list", m_unsorted_minimal_docs_list.bytes(),
          total_bytes, m_completions.size());

    print("dictionary", m_dictionary.bytes(), total_bytes,
          m_completions.size());
    print_bps("data", m_dictionary.data_bytes(), m_dictionary.size());
    print_bps("pointers", m_dictionary.pointer_bytes(), m_dictionary.size());

    print("inverted index", m_inverted_index.bytes(), total_bytes,
          m_completions.size());
    print_bpi("data", m_inverted_index.data_bytes(),
              m_inverted_index.num_integers());
    print_bpi("pointers", m_inverted_index.pointer_bytes(),
              m_inverted_index.num_integers());
    print("forward index", m_forward_index.bytes(), total_bytes,
          m_completions.size());
    std::cout << "\tavg. number of terms per completion: "
              << static_cast<double>(m_forward_index.num_integers()) /
                     m_completions.size()
              << std::endl;
    print_bpi("data", m_forward_index.data_bytes(),
              m_forward_index.num_integers());
    print_bpi("pointers", m_forward_index.pointer_bytes(),
              m_forward_index.num_integers());
}

template <typename Completions, typename Dictionary, typename InvertedIndex>
void autocomplete2<Completions, Dictionary, InvertedIndex>::print_stats()
    const {
    size_t total_bytes = bytes();
    std::cout << "using " << essentials::convert(total_bytes, essentials::MiB)
              << " [MiB]: "
              << static_cast<double>(total_bytes) / m_completions.size()
              << " [bytes per completion] " << std::endl;

    print("completions", m_completions.bytes(), total_bytes,
          m_completions.size());
    print_bps("data", m_completions.data_bytes(), m_completions.size());
    print_bps("pointers", m_completions.pointer_bytes(), m_completions.size());

    print("unsorted docs list", m_unsorted_docs_list.bytes(), total_bytes,
          m_completions.size());
    print("unsorted minimal docs list", m_unsorted_minimal_docs_list.bytes(),
          total_bytes, m_completions.size());

    print("dictionary", m_dictionary.bytes(), total_bytes,
          m_completions.size());
    print_bps("data", m_dictionary.data_bytes(), m_dictionary.size());
    print_bps("pointers", m_dictionary.pointer_bytes(), m_dictionary.size());

    print("inverted index", m_inverted_index.bytes(), total_bytes,
          m_completions.size());
    print_bpi("data", m_inverted_index.data_bytes(),
              m_inverted_index.num_integers());
    print_bpi("pointers", m_inverted_index.pointer_bytes(),
              m_inverted_index.num_integers());

    print("map from docid to lexid", m_docid_to_lexid.bytes(), total_bytes,
          m_completions.size());
}

template <typename Completions, typename Dictionary, typename InvertedIndex>
void autocomplete3<Completions, Dictionary, InvertedIndex>::print_stats()
    const {
    size_t total_bytes = bytes();
    std::cout << "using " << essentials::convert(total_bytes, essentials::MiB)
              << " [MiB]: "
              << static_cast<double>(total_bytes) / m_completions.size()
              << " [bytes per completion] " << std::endl;

    print("completions", m_completions.bytes(), total_bytes,
          m_completions.size());
    print("unsorted docs list", m_unsorted_docs_list.bytes(), total_bytes,
          m_completions.size());
    print("dictionary", m_dictionary.bytes(), total_bytes,
          m_completions.size());
    print("inverted index", m_inverted_index.bytes(), total_bytes,
          m_completions.size());
    print_bpi("data", m_inverted_index.data_bytes(),
              m_inverted_index.num_integers());
    print_bpi("pointers", m_inverted_index.pointer_bytes(),
              m_inverted_index.num_integers());
    print("map from docid to lexid", m_docid_to_lexid.bytes(), total_bytes,
          m_completions.size());
}

template <typename Completions, typename Dictionary,
          typename BlockedInvertedIndex>
void autocomplete4<Completions, Dictionary, BlockedInvertedIndex>::print_stats()
    const {
    size_t total_bytes = bytes();
    std::cout << "using " << essentials::convert(total_bytes, essentials::MiB)
              << " [MiB]: "
              << static_cast<double>(total_bytes) / m_completions.size()
              << " [bytes per completion] " << std::endl;

    print("completions", m_completions.bytes(), total_bytes,
          m_completions.size());
    print("unsorted docs list", m_unsorted_docs_list.bytes(), total_bytes,
          m_completions.size());
    print("dictionary", m_dictionary.bytes(), total_bytes,
          m_completions.size());

    print("blocked inverted index", m_inverted_index.bytes(), total_bytes,
          m_completions.size());
    std::cout << "  num blocks: " << m_inverted_index.num_blocks() << std::endl;
    print_bpi("blocks", m_inverted_index.blocks_bytes(),
              m_inverted_index.num_integers());
    print_bpi("pointers", m_inverted_index.pointers_bytes(),
              m_inverted_index.num_integers());
    print_bpi("docs", m_inverted_index.docs_bytes(),
              m_inverted_index.num_integers());
    print_bpi("terms", m_inverted_index.terms_bytes(),
              m_inverted_index.num_integers());
    print_bpi("offsets", m_inverted_index.offsets_bytes(),
              m_inverted_index.num_integers());

    print("map from docid to lexid", m_docid_to_lexid.bytes(), total_bytes,
          m_completions.size());
}

}  // namespace autocomplete