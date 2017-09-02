#pragma once

#include <atomic>

namespace snow {
    class SeqGenerator {
    public:
        static SeqGenerator& instance() {
            static SeqGenerator instance;
            return instance;
        }

        SeqGenerator(const SeqGenerator&) = delete;
        ~SeqGenerator() = default;
        void operator=(const SeqGenerator&) = delete;

        uint32_t get() {
            return m_seq++;
        }


    private:
        SeqGenerator() = default;
        std::atomic_uint m_seq;
    };
}