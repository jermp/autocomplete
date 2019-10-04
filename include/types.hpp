#pragma once

#include "completion_trie.hpp"
#include "fc_dictionary.hpp"
#include "integer_fc_dictionary.hpp"
#include "uint_vec.hpp"
#include "unsorted_list.hpp"
#include "uncompressed_list.hpp"
#include "forward_index.hpp"
#include "inverted_index.hpp"
#include "autocomplete.hpp"
#include "autocomplete2.hpp"

#include "succinct_rmq/cartesian_tree.hpp"

namespace autocomplete {

typedef uint_vec<uint32_t> uint32_vec;
typedef uint_vec<uint64_t> uint64_vec;

typedef completion_trie<uint64_vec, uint32_vec, uint32_vec, uint32_vec>
    uint64_completion_trie;

static const uint32_t BucketSize = 16;
typedef fc_dictionary<BucketSize, uint32_vec> fc_dictionary_B16;
typedef integer_fc_dictionary<BucketSize, uint64_vec> integer_fc_dictionary_B16;

typedef unsorted_list<uint32_vec, cartesian_tree> unsorted_list_succinct_rmq;

typedef uncompressed_list<uint32_t> uncompressed_list32_t;

typedef forward_index<uncompressed_list32_t, uint64_vec>
    uncompressed_forward_index;
typedef inverted_index<uncompressed_list32_t, uint64_vec>
    uncompressed_inverted_index;

typedef autocomplete<uint64_completion_trie, unsorted_list_succinct_rmq,
                     fc_dictionary_B16, uncompressed_inverted_index,
                     uncompressed_forward_index>
    uncompressed_autocomplete_type;

typedef autocomplete2<integer_fc_dictionary_B16, unsorted_list_succinct_rmq,
                      fc_dictionary_B16, uncompressed_inverted_index>
    uncompressed_autocomplete_type2;

}  // namespace autocomplete