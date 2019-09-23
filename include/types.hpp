#pragma once

#include "completion_trie.hpp"
#include "uint32_vec.hpp"

namespace autocomplete {

typedef completion_trie<uint32_vec, uint32_vec, uint32_vec, uint32_vec>
    uint32_completion_trie;

}