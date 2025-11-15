#ifndef MORPHEUS_PREFETCHER_H
#define MORPHEUS_PREFETCHER_H

#include <cstdint>
#include <cstddef>

namespace morpheus {

enum class PrefetchStrategy {
    None,
    Sequential,
    Strided,
    IMA
};

class Prefetcher {
public:
    virtual ~Prefetcher() = default;
    
    // Prefetch a memory address
    virtual void prefetch(const void* addr) = 0;
    
    // Get the strategy type
    virtual PrefetchStrategy getStrategy() const = 0;
    
    // Configure the prefetcher
    virtual void configure(size_t distance = 1, size_t stride = 64) = 0;
    
    // Reset internal state
    virtual void reset() = 0;
};

} // namespace morpheus

#endif // MORPHEUS_PREFETCHER_H