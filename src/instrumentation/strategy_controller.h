#ifndef MORPHEUS_STRATEGY_CONTROLLER_H
#define MORPHEUS_STRATEGY_CONTROLLER_H

#include <string>
#include <memory>
#include <unistd.h> // for close()
#include "../engine/monitoring/performance_monitor.h"
#include "../engine/prefetch/prefetcher.h"

namespace morpheus {

// Forward declarations for concrete prefetchers to avoid requiring their full headers here.
class SequentialPrefetcher;
class StridedPrefetcher;
class IMAPrefetcher;

class StrategyController {
public:
    StrategyController();
    ~StrategyController();
    
    // Initialize the controller
    bool initialize();
    
    // Update strategy based on current performance
    void updateStrategy(ExecutionPhase phase);
    
    // Get current prefetcher
    std::shared_ptr<Prefetcher> getCurrentPrefetcher() const { return current_prefetcher_; }
    
    // Get current strategy
    PrefetchStrategy getCurrentStrategy() const;
    
    // Communication with PIN tool
    bool updateSharedMemoryStrategy();

private:
    std::shared_ptr<Prefetcher> current_prefetcher_;
    std::shared_ptr<SequentialPrefetcher> sequential_prefetcher_;
    std::shared_ptr<StridedPrefetcher> strided_prefetcher_;
    std::shared_ptr<IMAPrefetcher> ima_prefetcher_;
    
    ExecutionPhase last_phase_;
    int shm_fd_;
    void* shared_memory_;
    
    bool setupSharedMemory();
};

// Inline simple implementations to satisfy linkage and keep the header self-contained.
// These are minimal stubs; replace with real implementations in a .cpp file as needed.

inline StrategyController::StrategyController()
    : current_prefetcher_(nullptr),
      sequential_prefetcher_(nullptr),
      strided_prefetcher_(nullptr),
      ima_prefetcher_(nullptr),
      last_phase_(),
      shm_fd_(-1),
      shared_memory_(nullptr)
{}

inline StrategyController::~StrategyController()
{
    if (shm_fd_ != -1) {
        close(shm_fd_);
        shm_fd_ = -1;
    }
    // Note: shared_memory_ cleanup (munmap) is omitted here because size is unknown;
    // add proper cleanup in the real implementation.
}

inline bool StrategyController::initialize()
{
    // Minimal initialization; real code should construct or configure prefetchers.
    return true;
}

inline void StrategyController::updateStrategy(ExecutionPhase /*phase*/)
{
    // Minimal stub; real logic to update current_prefetcher_ goes here.
}

inline PrefetchStrategy StrategyController::getCurrentStrategy() const
{
    // Return a default-constructed strategy if none is set; replace with real query as needed.
    return PrefetchStrategy();
}

inline bool StrategyController::updateSharedMemoryStrategy()
{
    // Stub: implement communication with PIN tool / shared memory here.
    return false;
}

inline bool StrategyController::setupSharedMemory()
{
    // Stub: implement shared memory setup here.
    return false;
}

} // namespace morpheus

#endif // MORPHEUS_STRATEGY_CONTROLLER_H