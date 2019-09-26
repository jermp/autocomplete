#pragma once

#include "completion_trie.hpp"
#include "fc_dictionary.hpp"
#include "uint32_vec.hpp"
#include "succinct_rmq/cartesian_tree.hpp"

namespace autocomplete {

typedef completion_trie<uint32_vec, uint32_vec, uint32_vec, uint32_vec>
    uint32_completion_trie;

static const uint32_t BucketSize = 16;
typedef fc_dictionary<BucketSize, uint32_vec> fc_dictionary_B16;

typedef cartesian_tree succinct_rmq;

}  // namespace autocomplete