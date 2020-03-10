#pragma once

#include <vector>
#include "util_types.hpp"

namespace autocomplete {

struct nop_probe {
    inline void start(uint64_t) {}
    inline void stop(uint64_t) {}
};

struct timer_probe {
    timer_probe(uint64_t n)
        : m_timers(n) {}

    inline void start(uint64_t i) {
        assert(i < m_timers.size());
        m_timers[i].start();
    }

    inline void stop(uint64_t i) {
        assert(i < m_timers.size());
        m_timers[i].stop();
    }

    timer_type const& get(uint64_t i) {
        assert(i < m_timers.size());
        return m_timers[i];
    }

private:
    std::vector<timer_type> m_timers;
};

}  // namespace autocomplete
