#ifndef MORPHEUS_STRIDED_PREFETCHER_H
#define MORPHEUS_STRIDED_PREFETCHER_H

#include "prefetcher.h"
#include <xmmintrin.h>
#include <vector>
#include <queue>
#include <cstdint>

namespace morpheus {

class StridedPrefetcher : public Prefetcher {
public:
    StridedPrefetcher() 
        : prefetch_distance_(1), 
          cache_line_size_(64),
          detected_stride_(0) {}
    
    ~StridedPrefetcher() override = default;
    
    void prefetch(const void* addr) override {
        detectStride(addr);
        
        if (detected_stride_ > 0) {
            const char* base_addr = static_cast<const char*>(addr);
            
            for (size_t i = 1; i <= prefetch_distance_; ++i) {
                const char* prefetch_addr = base_addr + i * detected_stride_;
                _mm_prefetch(prefetch_addr, _MM_HINT_T0);
            }
        } else {
            // Fall back to sequential if no stride detected
            const char* base_addr = static_cast<const char*>(addr);
            for (size_t i = 1; i <= prefetch_distance_; ++i) {
                const char* prefetch_addr = base_addr + i * cache_line_size_;
                _mm_prefetch(prefetch_addr, _MM_HINT_T0);
            }
        }
    }
    
    PrefetchStrategy getStrategy() const override { 
        return PrefetchStrategy::Strided; 
    }
    
    void configure(size_t distance = 1, size_t stride = 64) override {
        prefetch_distance_ = distance;
        cache_line_size_ = stride;
    }
    
    void reset() override {
        detected_stride_ = 0;
        while (!recent_addresses_.empty()) {
            recent_addresses_.pop();
        }
    }
    
    void detectStride(const void* addr) {
        recent_addresses_.push(addr);
        
        if (recent_addresses_.size() < HISTORY_SIZE) {
            return;
        }
        
        // Calculate strides between consecutive addresses
        std::vector<int64_t> strides;
        const void* prev_addr = nullptr;
        
        auto queue_copy = recent_addresses_;
        while (!queue_copy.empty()) {
            const void* current_addr = queue_copy.front();
            queue_copy.pop();
            
            if (prev_addr != nullptr) {
                int64_t stride = static_cast<const char*>(current_addr) - 
                                static_cast<const char*>(prev_addr);
                strides.push_back(stride);
            }
            prev_addr = current_addr;
        }
        
        // Check if we have a consistent stride
        if (!strides.empty()) {
            int64_t first_stride = strides[0];
            bool consistent = true;
            
            for (size_t i = 1; i < strides.size(); ++i) {
                if (strides[i] != first_stride) {
                    consistent = false;
                    break;
                }
            }
            
            if (consistent && first_stride > 0) {
                detected_stride_ = static_cast<size_t>(first_stride);
            }
        }
        
        // Maintain history size
        if (recent_addresses_.size() > HISTORY_SIZE) {
            recent_addresses_.pop();
        }
    }

private:
    size_t prefetch_distance_;
    size_t cache_line_size_;
    size_t detected_stride_;
    std::queue<const void*> recent_addresses_;
    static constexpr size_t HISTORY_SIZE = 8;
};

} // namespace morpheus

#endif // MORPHEUS_STRIDED_PREFETCHER_H