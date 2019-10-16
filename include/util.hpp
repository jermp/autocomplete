#pragma once

#pragma once

#include <string.h>
#include <sys/time.h>
#include <cassert>
#include <ctime>
#include <iostream>
#include <locale>
#include <vector>

#include <smmintrin.h>
#include <xmmintrin.h>

#include "../external/essentials/include/essentials.hpp"

#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define NOINLINE __attribute__((noinline))
#define ALWAYSINLINE __attribute__((always_inline))

namespace autocomplete {

// NOTE: assume 32 bits are enough to store
// both a term_id and a doc_id
typedef uint32_t id_type;

namespace global {
static const id_type invalid_term_id = id_type(-1);
static const id_type terminator = 0;
static const uint64_t not_found = uint64_t(-1);
static const uint64_t linear_scan_threshold = 8;
}  // namespace global

namespace util {

template <typename S>
uint64_t find(S const& sequence, uint64_t id, uint64_t lo, uint64_t hi) {
    while (lo <= hi) {
        if (hi - lo <= global::linear_scan_threshold) {
            auto it = sequence.at(lo);
            for (uint64_t pos = lo; pos != hi; ++pos, ++it) {
                if (*it == id) {
                    return pos;
                }
            }
        }
        uint64_t pos = lo + ((hi - lo) >> 1);
        uint64_t val = sequence.access(pos);
        if (val == id) {
            return pos;
        } else if (val > id) {
            hi = pos - 1;
        } else {
            lo = pos + 1;
        }
    }
    return global::not_found;
}

template <typename S>
uint64_t next_geq(S const& sequence, uint64_t lower_bound, uint64_t lo,
                  uint64_t hi) {
    while (lo <= hi) {
        if (hi - lo <= global::linear_scan_threshold) {
            auto it = sequence.at(lo);
            for (uint64_t pos = lo; pos <= hi; ++pos, ++it) {
                if (*it >= lower_bound) {
                    return pos;
                }
            }
            break;
        }

        uint64_t pos = (lo + hi) / 2;
        uint64_t val = sequence.access(pos);

        if (val > lower_bound) {
            hi = pos != 0 ? pos - 1 : 0;
            if (lower_bound > sequence.access(hi)) {
                return pos;
            }
        } else if (val < lower_bound) {
            lo = pos + 1;
        } else {
            return pos;
        }
    }

    return global::not_found;
}

}  // namespace util

namespace tables {
const uint8_t select_in_byte[2048] = {
    8, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3,
    0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0,
    1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1,
    0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0,
    2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2,
    0, 1, 0, 7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0,
    1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1,
    0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0,
    3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0,
    1, 0, 2, 0, 1, 0, 8, 8, 8, 1, 8, 2, 2, 1, 8, 3, 3, 1, 3, 2, 2, 1, 8, 4, 4,
    1, 4, 2, 2, 1, 4, 3, 3, 1, 3, 2, 2, 1, 8, 5, 5, 1, 5, 2, 2, 1, 5, 3, 3, 1,
    3, 2, 2, 1, 5, 4, 4, 1, 4, 2, 2, 1, 4, 3, 3, 1, 3, 2, 2, 1, 8, 6, 6, 1, 6,
    2, 2, 1, 6, 3, 3, 1, 3, 2, 2, 1, 6, 4, 4, 1, 4, 2, 2, 1, 4, 3, 3, 1, 3, 2,
    2, 1, 6, 5, 5, 1, 5, 2, 2, 1, 5, 3, 3, 1, 3, 2, 2, 1, 5, 4, 4, 1, 4, 2, 2,
    1, 4, 3, 3, 1, 3, 2, 2, 1, 8, 7, 7, 1, 7, 2, 2, 1, 7, 3, 3, 1, 3, 2, 2, 1,
    7, 4, 4, 1, 4, 2, 2, 1, 4, 3, 3, 1, 3, 2, 2, 1, 7, 5, 5, 1, 5, 2, 2, 1, 5,
    3, 3, 1, 3, 2, 2, 1, 5, 4, 4, 1, 4, 2, 2, 1, 4, 3, 3, 1, 3, 2, 2, 1, 7, 6,
    6, 1, 6, 2, 2, 1, 6, 3, 3, 1, 3, 2, 2, 1, 6, 4, 4, 1, 4, 2, 2, 1, 4, 3, 3,
    1, 3, 2, 2, 1, 6, 5, 5, 1, 5, 2, 2, 1, 5, 3, 3, 1, 3, 2, 2, 1, 5, 4, 4, 1,
    4, 2, 2, 1, 4, 3, 3, 1, 3, 2, 2, 1, 8, 8, 8, 8, 8, 8, 8, 2, 8, 8, 8, 3, 8,
    3, 3, 2, 8, 8, 8, 4, 8, 4, 4, 2, 8, 4, 4, 3, 4, 3, 3, 2, 8, 8, 8, 5, 8, 5,
    5, 2, 8, 5, 5, 3, 5, 3, 3, 2, 8, 5, 5, 4, 5, 4, 4, 2, 5, 4, 4, 3, 4, 3, 3,
    2, 8, 8, 8, 6, 8, 6, 6, 2, 8, 6, 6, 3, 6, 3, 3, 2, 8, 6, 6, 4, 6, 4, 4, 2,
    6, 4, 4, 3, 4, 3, 3, 2, 8, 6, 6, 5, 6, 5, 5, 2, 6, 5, 5, 3, 5, 3, 3, 2, 6,
    5, 5, 4, 5, 4, 4, 2, 5, 4, 4, 3, 4, 3, 3, 2, 8, 8, 8, 7, 8, 7, 7, 2, 8, 7,
    7, 3, 7, 3, 3, 2, 8, 7, 7, 4, 7, 4, 4, 2, 7, 4, 4, 3, 4, 3, 3, 2, 8, 7, 7,
    5, 7, 5, 5, 2, 7, 5, 5, 3, 5, 3, 3, 2, 7, 5, 5, 4, 5, 4, 4, 2, 5, 4, 4, 3,
    4, 3, 3, 2, 8, 7, 7, 6, 7, 6, 6, 2, 7, 6, 6, 3, 6, 3, 3, 2, 7, 6, 6, 4, 6,
    4, 4, 2, 6, 4, 4, 3, 4, 3, 3, 2, 7, 6, 6, 5, 6, 5, 5, 2, 6, 5, 5, 3, 5, 3,
    3, 2, 6, 5, 5, 4, 5, 4, 4, 2, 5, 4, 4, 3, 4, 3, 3, 2, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 3, 8, 8, 8, 8, 8, 8, 8, 4, 8, 8, 8, 4, 8, 4, 4, 3,
    8, 8, 8, 8, 8, 8, 8, 5, 8, 8, 8, 5, 8, 5, 5, 3, 8, 8, 8, 5, 8, 5, 5, 4, 8,
    5, 5, 4, 5, 4, 4, 3, 8, 8, 8, 8, 8, 8, 8, 6, 8, 8, 8, 6, 8, 6, 6, 3, 8, 8,
    8, 6, 8, 6, 6, 4, 8, 6, 6, 4, 6, 4, 4, 3, 8, 8, 8, 6, 8, 6, 6, 5, 8, 6, 6,
    5, 6, 5, 5, 3, 8, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3, 8, 8, 8, 8,
    8, 8, 8, 7, 8, 8, 8, 7, 8, 7, 7, 3, 8, 8, 8, 7, 8, 7, 7, 4, 8, 7, 7, 4, 7,
    4, 4, 3, 8, 8, 8, 7, 8, 7, 7, 5, 8, 7, 7, 5, 7, 5, 5, 3, 8, 7, 7, 5, 7, 5,
    5, 4, 7, 5, 5, 4, 5, 4, 4, 3, 8, 8, 8, 7, 8, 7, 7, 6, 8, 7, 7, 6, 7, 6, 6,
    3, 8, 7, 7, 6, 7, 6, 6, 4, 7, 6, 6, 4, 6, 4, 4, 3, 8, 7, 7, 6, 7, 6, 6, 5,
    7, 6, 6, 5, 6, 5, 5, 3, 7, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 5, 8, 8, 8,
    8, 8, 8, 8, 5, 8, 8, 8, 5, 8, 5, 5, 4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 6, 8, 8, 8, 8, 8, 8, 8, 6, 8, 8, 8, 6, 8, 6, 6, 4, 8, 8, 8, 8, 8,
    8, 8, 6, 8, 8, 8, 6, 8, 6, 6, 5, 8, 8, 8, 6, 8, 6, 6, 5, 8, 6, 6, 5, 6, 5,
    5, 4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 8, 8, 8, 8, 8, 8, 8,
    7, 8, 8, 8, 7, 8, 7, 7, 4, 8, 8, 8, 8, 8, 8, 8, 7, 8, 8, 8, 7, 8, 7, 7, 5,
    8, 8, 8, 7, 8, 7, 7, 5, 8, 7, 7, 5, 7, 5, 5, 4, 8, 8, 8, 8, 8, 8, 8, 7, 8,
    8, 8, 7, 8, 7, 7, 6, 8, 8, 8, 7, 8, 7, 7, 6, 8, 7, 7, 6, 7, 6, 6, 4, 8, 8,
    8, 7, 8, 7, 7, 6, 8, 7, 7, 6, 7, 6, 6, 5, 8, 7, 7, 6, 7, 6, 6, 5, 7, 6, 6,
    5, 6, 5, 5, 4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 5, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    6, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 6, 8, 8, 8, 8, 8, 8, 8, 6,
    8, 8, 8, 6, 8, 6, 6, 5, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 7, 8, 8, 8, 8, 8, 8, 8, 7, 8, 8, 8, 7, 8, 7, 7, 5, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 8, 8, 8, 8, 8, 8, 8, 7, 8, 8, 8, 7,
    8, 7, 7, 6, 8, 8, 8, 8, 8, 8, 8, 7, 8, 8, 8, 7, 8, 7, 7, 6, 8, 8, 8, 7, 8,
    7, 7, 6, 8, 7, 7, 6, 7, 6, 6, 5, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 6, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    7, 8, 8, 8, 8, 8, 8, 8, 7, 8, 8, 8, 7, 8, 7, 7, 6, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7};
}

namespace util {

template <typename T>
inline void prefetch(T const* ptr) {
    _mm_prefetch(reinterpret_cast<const char*>(ptr), _MM_HINT_T0);
}

inline uint8_t msb(uint64_t x) {
    assert(x);
    unsigned long ret = -1U;
    if (x) {
        ret = (unsigned long)(63 - __builtin_clzll(x));
    }
    return (uint8_t)ret;
}

inline bool bsr64(unsigned long* const index, const uint64_t mask) {
    if (mask) {
        *index = (unsigned long)(63 - __builtin_clzll(mask));
        return true;
    } else {
        return false;
    }
}

inline uint8_t msb(uint64_t x, unsigned long& ret) {
    return bsr64(&ret, x);
}

inline uint8_t lsb(uint64_t x, unsigned long& ret) {
    if (x) {
        ret = (unsigned long)__builtin_ctzll(x);
        return true;
    }
    return false;
}

inline uint8_t lsb(uint64_t x) {
    assert(x);
    unsigned long ret = -1U;
    lsb(x, ret);
    return (uint8_t)ret;
}

inline uint64_t ceil_log2(const uint64_t x) {
    return (x > 1) ? msb(x - 1) + 1 : 0;
}

inline uint64_t floor_log2(const uint64_t x) {
    return (x > 1) ? msb(x) : 0;
}

static const uint64_t ones_step_4 = 0x1111111111111111ULL;
static const uint64_t ones_step_8 = 0x0101010101010101ULL;
static const uint64_t ones_step_9 = 1ULL << 0 | 1ULL << 9 | 1ULL << 18 |
                                    1ULL << 27 | 1ULL << 36 | 1ULL << 45 |
                                    1ULL << 54;
static const uint64_t msbs_step_8 = 0x80ULL * ones_step_8;
static const uint64_t msbs_step_9 = 0x100ULL * ones_step_9;
static const uint64_t incr_step_8 =
    0x80ULL << 56 | 0x40ULL << 48 | 0x20ULL << 40 | 0x10ULL << 32 |
    0x8ULL << 24 | 0x4ULL << 16 | 0x2ULL << 8 | 0x1;
static const uint64_t inv_count_step_9 = 1ULL << 54 | 2ULL << 45 | 3ULL << 36 |
                                         4ULL << 27 | 5ULL << 18 | 6ULL << 9 |
                                         7ULL;

static const uint64_t magic_mask_1 = 0x5555555555555555ULL;
static const uint64_t magic_mask_2 = 0x3333333333333333ULL;
static const uint64_t magic_mask_3 = 0x0F0F0F0F0F0F0F0FULL;
static const uint64_t magic_mask_4 = 0x00FF00FF00FF00FFULL;
static const uint64_t magic_mask_5 = 0x0000FFFF0000FFFFULL;
static const uint64_t magic_mask_6 = 0x00000000FFFFFFFFULL;

inline uint64_t leq_step_8(uint64_t x, uint64_t y) {
    return ((((y | msbs_step_8) - (x & ~msbs_step_8)) ^ (x ^ y)) &
            msbs_step_8) >>
           7;
}

inline uint64_t uleq_step_9(uint64_t x, uint64_t y) {
    return (((((y | msbs_step_9) - (x & ~msbs_step_9)) | (x ^ y)) ^ (x & ~y)) &
            msbs_step_9) >>
           8;
}

inline uint64_t byte_counts(uint64_t x) {
    x = x - ((x & 0xa * ones_step_4) >> 1);
    x = (x & 3 * ones_step_4) + ((x >> 2) & 3 * ones_step_4);
    x = (x + (x >> 4)) & 0x0f * ones_step_8;
    return x;
}

inline uint64_t bytes_sum(uint64_t x) {
    return x * ones_step_8 >> 56;
}

inline uint64_t byteswap64(uint64_t value) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_bswap64(value);
#else
#error Unsupported platform
#endif
}

inline uint64_t reverse_bytes(uint64_t x) {
#if USE_INTRINSICS
    return byteswap64(x);
#else
    x = ((x >> 8) & magic_mask_4) | ((x & magic_mask_4) << 8);
    x = ((x >> 16) & magic_mask_5) | ((x & magic_mask_5) << 16);
    x = ((x >> 32)) | ((x) << 32);
    return x;
#endif
}

inline uint64_t reverse_bits(uint64_t x) {
    x = ((x >> 1) & magic_mask_1) | ((x & magic_mask_1) << 1);
    x = ((x >> 2) & magic_mask_2) | ((x & magic_mask_2) << 2);
    x = ((x >> 4) & magic_mask_3) | ((x & magic_mask_3) << 4);
    return reverse_bytes(x);
}

inline uint64_t popcount(uint64_t x) {
#if USE_INTRINSICS
    return uint64_t(_mm_popcnt_u64(x));
#else
    return bytes_sum(byte_counts(x));
#endif
}

// NOTE: this is the select-in-word algorithm presented in
// "A Fast x86 Implementation of Select" by
// P. Pandey, M. A. Bender, and R. Johnson
// the algorithm uses only four x86 machine instructions,
// two of which were introduced in Intel’s Haswell CPUs in 2013
// source: https://github.com/splatlab/rankselect/blob/master/popcount.h
inline uint64_t select64_pdep_tzcnt(uint64_t x, const uint64_t k) {
    uint64_t i = 1ULL << k;
    asm("pdep %[x], %[mask], %[x]" : [ x ] "+r"(x) : [ mask ] "r"(i));
    asm("tzcnt %[bit], %[index]" : [ index ] "=r"(i) : [ bit ] "g"(x) : "cc");
    return i;
}

inline uint64_t select_in_word(const uint64_t x, const uint64_t k) {
    assert(k < popcount(x));
#if USE_PDEP
    return select64_pdep_tzcnt(x, k);
#else
    uint64_t byte_sums = byte_counts(x) * ones_step_8;
    const uint64_t k_step_8 = k * ones_step_8;
    const uint64_t geq_k_step_8 =
        (((k_step_8 | msbs_step_8) - byte_sums) & msbs_step_8);
    const uint64_t place = popcount(geq_k_step_8) * 8;
    const uint64_t byte_rank =
        k - (((byte_sums << 8) >> place) & uint64_t(0xFF));
    return place +
           tables::select_in_byte[((x >> place) & 0xFF) | (byte_rank << 8)];
#endif
}

template <typename IntType1, typename IntType2>
inline IntType1 ceil_div(IntType1 dividend, IntType2 divisor) {
    IntType1 d = IntType1(divisor);
    return IntType1(dividend + d - 1) / d;
}
}  // namespace util

}  // namespace autocomplete