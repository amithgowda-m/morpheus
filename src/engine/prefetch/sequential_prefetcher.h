#ifndef MORPHEUS_SEQUENTIAL_PREFETCHER_H
#define MORPHEUS_SEQUENTIAL_PREFETCHER_H

#include "prefetcher.h"
#include <xmmintrin.h>

namespace morpheus {

class SequentialPrefetcher : public Prefetcher {
public:
    SequentialPrefetcher() : prefetch_distance_(1), cache_line_size_(64) {}
    ~SequentialPrefetcher() override = default;
    
    void prefetch(const void* addr) override {
        const char* base_addr = static_cast<const char*>(addr);
        
        // Prefetch multiple cache lines ahead
        for (size_t i = 1; i <= prefetch_distance_; ++i) {
            const char* prefetch_addr = base_addr + i * cache_line_size_;
            _mm_prefetch(prefetch_addr, _MM_HINT_T0);
        }
    }
    
    PrefetchStrategy getStrategy() const override { 
        return PrefetchStrategy::Sequential; 
    }
    
    void configure(size_t distance = 1, size_t stride = 64) override {
        prefetch_distance_ = distance;
        cache_line_size_ = stride;
    }
    
    void reset() override {
        // No state to reset for sequential prefetcher
    }

private:
    size_t prefetch_distance_;
    size_t cache_line_size_;
};

} // namespace morpheus

#endif // MORPHEUS_SEQUENTIAL_PREFETCHER_H