#include "autocomplete.hpp"
#include "autocomplete2.hpp"
#include "autocomplete3.hpp"
#include "autocomplete4.hpp"

namespace autocomplete {

void print(std::string const& what, size_t bytes, size_t total_bytes) {
    std::cout << "  " << what << ": " << convert(bytes, essentials::GiB)
              << " [GiB] ";
    std::cout << "(" << (bytes * 100.0) / total_bytes << "%)" << std::endl;
}

void print_bpi(std::string const& what, size_t bytes, size_t integers) {
    std::cout << '\t' << what << ": " << (bytes * 8.0) / integers << " [bpi]"
              << std::endl;
}

template <typename Completions, typename UnsortedDocsList, typename Dictionary,
          typename InvertedIndex, typename ForwardIndex>
void autocomplete<Completions, UnsortedDocsList, Dictionary, InvertedIndex,
                  ForwardIndex>::print_stats() const {
    size_t total_bytes = bytes();
    std::cout << "using " << convert(total_bytes, essentials::GiB) << " [GiB]"
              << std::endl;
    print("completions", m_completions.bytes(), total_bytes);
    print("unsorted docs list", m_unsorted_docs_list.bytes(), total_bytes);
    print("unsorted minimal docs list", m_unsorted_minimal_docs_list.bytes(),
          total_bytes);
    print("dictionary", m_dictionary.bytes(), total_bytes);
    print("inverted index", m_inverted_index.bytes(), total_bytes);
    print_bpi("data", m_inverted_index.data_bytes(),
              m_inverted_index.num_integers());
    print_bpi("pointers", m_inverted_index.pointer_bytes(),
              m_inverted_index.num_integers());
    print("forward index", m_forward_index.bytes(), total_bytes);
    print_bpi("data", m_forward_index.data_bytes(),
              m_forward_index.num_integers());
    print_bpi("pointers", m_forward_index.pointer_bytes(),
              m_forward_index.num_integers());
}

template <typename Completions, typename UnsortedDocsList, typename Dictionary,
          typename InvertedIndex>
void autocomplete2<Completions, UnsortedDocsList, Dictionary,
                   InvertedIndex>::print_stats() const {
    size_t total_bytes = bytes();
    std::cout << "using " << convert(total_bytes, essentials::GiB) << " [GiB]"
              << std::endl;
    print("completions", m_completions.bytes(), total_bytes);
    print("unsorted docs list", m_unsorted_docs_list.bytes(), total_bytes);
    print("unsorted minimal docs list", m_unsorted_minimal_docs_list.bytes(),
          total_bytes);
    print("dictionary", m_dictionary.bytes(), total_bytes);
    print("inverted index", m_inverted_index.bytes(), total_bytes);
    print_bpi("data", m_inverted_index.data_bytes(),
              m_inverted_index.num_integers());
    print_bpi("pointers", m_inverted_index.pointer_bytes(),
              m_inverted_index.num_integers());
    print("map from docid to lexid", essentials::vec_bytes(m_docid_to_lexid),
          total_bytes);
}

template <typename Completions, typename UnsortedDocsList, typename Dictionary,
          typename InvertedIndex>
void autocomplete3<Completions, UnsortedDocsList, Dictionary,
                   InvertedIndex>::print_stats() const {
    size_t total_bytes = bytes();
    std::cout << "using " << convert(total_bytes, essentials::GiB) << " [GiB]"
              << std::endl;
    print("completions", m_completions.bytes(), total_bytes);
    print("unsorted docs list", m_unsorted_docs_list.bytes(), total_bytes);
    print("dictionary", m_dictionary.bytes(), total_bytes);
    print("inverted index", m_inverted_index.bytes(), total_bytes);
    print_bpi("data", m_inverted_index.data_bytes(),
              m_inverted_index.num_integers());
    print_bpi("pointers", m_inverted_index.pointer_bytes(),
              m_inverted_index.num_integers());
    print("map from docid to lexid", essentials::vec_bytes(m_docid_to_lexid),
          total_bytes);
}

template <typename Completions, typename UnsortedDocsList, typename Dictionary,
          typename BlockedInvertedIndex>
void autocomplete4<Completions, UnsortedDocsList, Dictionary,
                   BlockedInvertedIndex>::print_stats() const {
    size_t total_bytes = bytes();
    std::cout << "using " << convert(total_bytes, essentials::GiB) << " [GiB]"
              << std::endl;
    print("completions", m_completions.bytes(), total_bytes);
    print("unsorted docs list", m_unsorted_docs_list.bytes(), total_bytes);
    print("dictionary", m_dictionary.bytes(), total_bytes);
    print("blocked inverted index", m_inverted_index.bytes(), total_bytes);
    print_bpi("data+pointers", m_inverted_index.bytes(),
              m_inverted_index.num_integers());
    print("map from docid to lexid", essentials::vec_bytes(m_docid_to_lexid),
          total_bytes);
}

}  // namespace autocomplete