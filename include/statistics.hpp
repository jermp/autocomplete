#include "autocomplete.hpp"

namespace autocomplete {

void print(std::string const& what, size_t bytes, size_t total_bytes) {
    std::cout << "  " << what << ": " << convert(bytes, essentials::GiB)
              << " [GiB] ";
    std::cout << "(" << (bytes * 100.0) / total_bytes << "%)" << std::endl;
}

template <typename CompletionTrie, typename UnsortedDocsList,
          typename Dictionary, typename InvertedIndex, typename ForwardIndex>
void autocomplete<CompletionTrie, UnsortedDocsList, Dictionary, InvertedIndex,
                  ForwardIndex>::print_stats() const {
    size_t total_bytes = bytes();
    std::cout << "using " << convert(total_bytes, essentials::GiB) << " [GiB]"
              << std::endl;
    print("completion trie", m_completion_trie.bytes(), total_bytes);
    print("unsorted docs list", m_unsorted_docs_list.bytes(), total_bytes);
    print("unsorted minimal docs list", m_unsorted_minimal_docs_list.bytes(),
          total_bytes);
    print("dictionary", m_dictionary.bytes(), total_bytes);
    print("inverted index", m_inverted_index.bytes(), total_bytes);
    print("forward index", m_forward_index.bytes(), total_bytes);
}

}  // namespace autocomplete