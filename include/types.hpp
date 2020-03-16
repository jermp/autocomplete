#pragma once

#include "completion_trie.hpp"
#include "fc_dictionary.hpp"
#include "integer_fc_dictionary.hpp"
#include "compact_forward_index.hpp"
#include "inverted_index.hpp"
#include "blocked_inverted_index.hpp"
#include "autocomplete.hpp"
#include "autocomplete2.hpp"
#include "autocomplete3.hpp"
#include "autocomplete4.hpp"
#include "compact_vector.hpp"
#include "ef/ef_sequence.hpp"
#include "ef/compact_ef.hpp"

namespace autocomplete {

typedef uint_vec<uint32_t> uint32_vec;
typedef uint_vec<uint64_t> uint64_vec;

typedef completion_trie<ef::ef_sequence, ef::ef_sequence, ef::ef_sequence,
                        ef::ef_sequence>
    ef_completion_trie;
typedef fc_dictionary<> fc_dictionary_type;
typedef integer_fc_dictionary<> integer_fc_dictionary_type;
typedef inverted_index<ef::compact_ef> ef_inverted_index;
typedef blocked_inverted_index<ef::compact_ef> ef_blocked_inverted_index;

/* compressed indexes */
typedef autocomplete<ef_completion_trie, fc_dictionary_type, ef_inverted_index,
                     compact_forward_index>
    ef_autocomplete_type1;

typedef autocomplete2<integer_fc_dictionary_type, fc_dictionary_type,
                      ef_inverted_index>
    ef_autocomplete_type2;

typedef autocomplete3<integer_fc_dictionary_type, fc_dictionary_type,
                      ef_inverted_index>
    ef_autocomplete_type3;

typedef autocomplete4<integer_fc_dictionary_type, fc_dictionary_type,
                      ef_blocked_inverted_index>
    ef_autocomplete_type4;

}  // namespace autocomplete