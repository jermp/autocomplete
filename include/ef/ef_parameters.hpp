#pragma once

namespace autocomplete {
namespace ef {

struct ef_parameters {
    ef_parameters()
        : ef_log_sampling0(9)
        , ef_log_sampling1(8) {}

    uint8_t ef_log_sampling0;
    uint8_t ef_log_sampling1;
};

struct ef_offsets {
    ef_offsets() {}

    ef_offsets(uint64_t base_offset, uint64_t universe, uint64_t n,
               ef_parameters const& params)
        : universe(universe)
        , n(n)
        , log_sampling0(params.ef_log_sampling0)
        , log_sampling1(params.ef_log_sampling1)

        , lower_bits(universe > n ? util::msb(universe / n) : 0)
        , mask((uint64_t(1) << lower_bits) - 1)
        // pad with a zero on both sides as sentinels
        , higher_bits_length(n + (universe >> lower_bits) + 2)
        , pointer_size(util::ceil_log2(higher_bits_length))
        , pointers0((higher_bits_length - n) >> log_sampling0)  // XXX
        , pointers1(n >> log_sampling1)

        , pointers0_offset(base_offset)
        , pointers1_offset(pointers0_offset + pointers0 * pointer_size)
        , higher_bits_offset(pointers1_offset + pointers1 * pointer_size)
        , lower_bits_offset(higher_bits_offset + higher_bits_length)
        , end(lower_bits_offset + n * lower_bits) {
        assert(n > 0);
    }

    uint64_t universe;
    uint64_t n;
    uint64_t log_sampling0;
    uint64_t log_sampling1;

    uint64_t lower_bits;
    uint64_t mask;
    uint64_t higher_bits_length;
    uint64_t pointer_size;
    uint64_t pointers0;
    uint64_t pointers1;

    uint64_t pointers0_offset;
    uint64_t pointers1_offset;
    uint64_t higher_bits_offset;
    uint64_t lower_bits_offset;
    uint64_t end;
};

}  // namespace ef
}  // namespace autocomplete