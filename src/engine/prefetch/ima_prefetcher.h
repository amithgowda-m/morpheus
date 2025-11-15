#ifndef MORPHEUS_IMA_PREFETCHER_H
#define MORPHEUS_IMA_PREFETCHER_H

#include "prefetcher.h"
#include <xmmintrin.h>
#include <vector>
#include <unordered_map>

namespace morpheus {

class IMAPrefetcher : public Prefetcher {
public:
    IMAPrefetcher() 
        : prefetch_distance_(1), 
          cache_line_size_(64) {}
    
    ~IMAPrefetcher() override = default;
    
    void prefetch(const void* addr) override {
        // Look for learned pointer chains starting from this address
        auto it = pointer_chains_.find(addr);
        if (it != pointer_chains_.end()) {
            const auto& chain = it->second;
            
            // Prefetch along the pointer chain
            for (size_t i = 0; i < chain.size() && i < prefetch_distance_; ++i) {
                _mm_prefetch(chain[i], _MM_HINT_T0);
            }
        } else {
            // Fallback: prefetch the next cache line
            const char* base_addr = static_cast<const char*>(addr);
            const char* prefetch_addr = base_addr + cache_line_size_;
            _mm_prefetch(prefetch_addr, _MM_HINT_T0);
        }
    }
    
    PrefetchStrategy getStrategy() const override { 
        return PrefetchStrategy::IMA; 
    }
    
    void configure(size_t distance = 1, size_t stride = 64) override {
        prefetch_distance_ = distance;
        cache_line_size_ = stride;
    }
    
    void reset() override {
        pointer_chains_.clear();
    }
    
    void learnPointerChain(const void* base_addr, const void* target_addr) {
        // Simple learning: record that base_addr often leads to target_addr
        auto& chain = pointer_chains_[base_addr];
        
        if (chain.size() < MAX_CHAIN_LENGTH) {
            chain.push_back(target_addr);
        } else {
            // Replace oldest entry
            chain.erase(chain.begin());
            chain.push_back(target_addr);
        }
    }

private:
    size_t prefetch_distance_;
    size_t cache_line_size_;
    
    std::unordered_map<const void*, std::vector<const void*>> pointer_chains_;
    static constexpr size_t MAX_CHAIN_LENGTH = 4;
};

} // namespace morpheus

#endif // MORPHEUS_IMA_PREFETCHER_H