#pragma once

#include "completion_trie.hpp"
#include "fc_dictionary.hpp"
#include "integer_fc_dictionary.hpp"
#include "uint_vec.hpp"
#include "unsorted_list.hpp"
#include "uncompressed_list.hpp"

#include "forward_index.hpp"
#include "compact_forward_index.hpp"
#include "delta_forward_index.hpp"

#include "inverted_index.hpp"
#include "blocked_inverted_index.hpp"

#include "autocomplete.hpp"
#include "autocomplete2.hpp"
#include "autocomplete3.hpp"
#include "autocomplete4.hpp"

#include "compact_vector.hpp"
#include "ef/ef_sequence.hpp"
#include "ef/compact_ef.hpp"
#include "succinct_rmq/cartesian_tree.hpp"

namespace autocomplete {

typedef uint_vec<uint32_t> uint32_vec;
typedef uint_vec<uint64_t> uint64_vec;

// typedef completion_trie<uint64_vec, uint32_vec, uint32_vec, uint32_vec>
//     uint64_completion_trie;

typedef completion_trie<ef::ef_sequence, ef::ef_sequence, ef::ef_sequence,
                        ef::ef_sequence>
    ef_completion_trie;

typedef fc_dictionary<> fc_dictionary_type;
typedef integer_fc_dictionary<> integer_fc_dictionary_type;

typedef unsorted_list<cartesian_tree> succinct_rmq;
typedef uncompressed_list<uint32_t> uncompressed_list32_t;

// typedef inverted_index<uncompressed_list32_t> uncompressed_inverted_index;
typedef inverted_index<ef::compact_ef> ef_inverted_index;

// typedef forward_index<uncompressed_list32_t> uncompressed_forward_index;
// typedef forward_index<ef::compact_ef> ef_forward_index;

// typedef blocked_inverted_index<uncompressed_list32_t>
//     uncompressed_blocked_inverted_index;
typedef blocked_inverted_index<ef::compact_ef> ef_blocked_inverted_index;

// typedef autocomplete<uint64_completion_trie, succinct_rmq,
// fc_dictionary_type,
//                      uncompressed_inverted_index, uncompressed_forward_index>
//     uncompressed_autocomplete_type;

// typedef autocomplete2<integer_fc_dictionary_type, succinct_rmq,
//                       fc_dictionary_type, uncompressed_inverted_index>
//     uncompressed_autocomplete_type2;

/* compressed indexes */
typedef autocomplete<ef_completion_trie, succinct_rmq, fc_dictionary_type,
                     ef_inverted_index, compact_forward_index>
    ef_autocomplete_type;

typedef autocomplete2<integer_fc_dictionary_type, succinct_rmq,
                      fc_dictionary_type, ef_inverted_index>
    ef_autocomplete_type2;

typedef autocomplete3<integer_fc_dictionary_type, succinct_rmq,
                      fc_dictionary_type, ef_inverted_index>
    ef_autocomplete_type3;

typedef autocomplete4<integer_fc_dictionary_type, succinct_rmq,
                      fc_dictionary_type, ef_blocked_inverted_index>
    ef_autocomplete_type4;
}  // namespace autocomplete