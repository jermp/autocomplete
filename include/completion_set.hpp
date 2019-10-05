#pragma once

namespace autocomplete {

struct completion_set {
    void resize(uint32_t k, uint32_t max_num_terms_per_completion) {
        m_sizes.resize(k);
        m_completions.resize(k);
        for (auto& c : m_completions) {
            c.resize(max_num_terms_per_completion);
        }
    }

    std::vector<completion_type>& completions() {
        return m_completions;
    }

    std::vector<uint8_t>& sizes() {
        return m_sizes;
    }

private:
    std::vector<uint8_t> m_sizes;
    std::vector<completion_type> m_completions;
};

}  // namespace autocomplete