#pragma once

namespace autocomplete {

namespace constants {

static const size_t POOL_SIZE = 1024;
static const uint32_t MAX_K = 15;
static const uint32_t MAX_NUM_TERMS_PER_QUERY = 64;
static_assert(MAX_NUM_TERMS_PER_QUERY < 256,
              "MAX_NUM_TERMS_PER_QUERY must be < 256");

}  // namespace constants

}  // namespace autocomplete