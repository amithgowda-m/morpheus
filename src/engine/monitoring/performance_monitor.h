#ifndef MORPHEUS_PERFORMANCE_MONITOR_H
#define MORPHEUS_PERFORMANCE_MONITOR_H

#include <cstdint>
#include <vector>
#include <string>
#include <linux/perf_event.h>
#include <sys/types.h>
#include <unordered_map>
#include "trained_classifier.h"

namespace morpheus {

// PerformanceSample holds per-sample counters and phase
struct PerformanceSample {
    uint64_t timestamp_ns;
    uint64_t instructions;
    uint64_t cycles;
    uint64_t l1_misses;
    uint64_t l2_misses;
    uint64_t l3_misses;
    uint64_t branch_misses;
    ExecutionPhase phase;
    
    // Derived metrics with safety checks
    double ipc() const { return cycles ? static_cast<double>(instructions) / cycles : 0.0; }
    double l1_miss_rate() const { return instructions ? static_cast<double>(l1_misses) / instructions : 0.0; }
    double l2_miss_rate() const { return instructions ? static_cast<double>(l2_misses) / instructions : 0.0; }
    double l3_miss_rate() const { return instructions ? static_cast<double>(l3_misses) / instructions : 0.0; }
    double branch_miss_rate() const { return instructions ? static_cast<double>(branch_misses) / instructions : 0.0; }
};

class PerformanceMonitor {
public:
    PerformanceMonitor();
    ~PerformanceMonitor();
    
    // Initialize performance counters
    bool initialize(uint64_t sample_interval_ms = 1);
    
    // Start/stop monitoring
    bool startMonitoring();
    bool stopMonitoring();
    
    // Get collected samples
    const std::vector<PerformanceSample>& getSamples() const { return samples_; }
    
    // Get current phase classification
    ExecutionPhase getCurrentPhase() const;
    
    // Reset samples
    void clearSamples() { samples_.clear(); }
    
    // Feature extraction for classification
    std::vector<double> extractFeatures() const;

private:
    std::vector<PerformanceSample> samples_;
    std::vector<int> perf_fds_;
    bool initialized_ = false;
    bool monitoring_ = false;
    uint64_t sample_interval_ms_ = 1;
    
    // perf_event_open file descriptors for different events
    int openPerfEvent(uint32_t type, uint64_t config);
    PerformanceSample readCounters();
};

} // namespace morpheus

#endif // MORPHEUS_PERFORMANCE_MONITOR_H