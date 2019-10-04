#include "autocomplete.hpp"
#include "autocomplete2.hpp"

namespace autocomplete {

void print(std::string const& what, size_t bytes, size_t total_bytes) {
    std::cout << "  " << what << ": " << convert(bytes, essentials::GiB)
              << " [GiB] ";
    std::cout << "(" << (bytes * 100.0) / total_bytes << "%)" << std::endl;
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
    print("forward index", m_forward_index.bytes(), total_bytes);
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
    print("map from docid to lexid", essentials::vec_bytes(m_docid_to_lexid),
          total_bytes);
}

}  // namespace autocomplete