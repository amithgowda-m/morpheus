#include "performance_monitor.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/ioctl.h>
#include <asm/unistd.h>

namespace morpheus {

static long perf_event_open(struct perf_event_attr* hw_event, pid_t pid,
                           int cpu, int group_fd, unsigned long flags) {
    return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

PerformanceMonitor::PerformanceMonitor() 
    : initialized_(false), monitoring_(false), sample_interval_ms_(1) {}

PerformanceMonitor::~PerformanceMonitor() {
    stopMonitoring();
    
    for (int fd : perf_fds_) {
        if (fd != -1) {
            close(fd);
        }
    }
}

bool PerformanceMonitor::initialize(uint64_t sample_interval_ms) {
    if (initialized_) {
        return true;
    }
    
    sample_interval_ms_ = sample_interval_ms;
    
    // Define events to monitor
    struct EventConfig {
        uint32_t type;
        uint64_t config;
    };
    
    std::vector<EventConfig> events = {
        {PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS},
        {PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES},
        {PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1D | 
                            (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                            (PERF_COUNT_HW_CACHE_RESULT_MISS << 16)},
        {PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_LL | 
                            (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                            (PERF_COUNT_HW_CACHE_RESULT_MISS << 16)},
        {PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES},  // L3 misses
        {PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES}
    };
    
    for (const auto& event : events) {
        int fd = openPerfEvent(event.type, event.config);
        if (fd == -1) {
            std::cerr << "Failed to open perf event: type=" << event.type 
                      << ", config=" << event.config << std::endl;
            // Continue with other events
        } else {
            perf_fds_.push_back(fd);
        }
    }
    
    if (perf_fds_.empty()) {
        std::cerr << "No perf events could be opened" << std::endl;
        return false;
    }
    
    initialized_ = true;
    return true;
}

int PerformanceMonitor::openPerfEvent(uint32_t type, uint64_t config) {
    struct perf_event_attr pe;
    std::memset(&pe, 0, sizeof(pe));
    
    pe.type = type;
    pe.size = sizeof(pe);
    pe.config = config;
    pe.disabled = 1;
    pe.exclude_kernel = 0;  // Include kernel events
    pe.exclude_hv = 1;      // Exclude hypervisor
    pe.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;
    
    int fd = perf_event_open(&pe, 0, -1, -1, 0);
    if (fd == -1) {
        return -1;
    }
    
    return fd;
}

bool PerformanceMonitor::startMonitoring() {
    if (!initialized_ || monitoring_) {
        return false;
    }
    
    for (int fd : perf_fds_) {
        if (ioctl(fd, PERF_EVENT_IOC_RESET, 0) == -1) {
            std::cerr << "Failed to reset perf event" << std::endl;
            return false;
        }
        
        if (ioctl(fd, PERF_EVENT_IOC_ENABLE, 0) == -1) {
            std::cerr << "Failed to enable perf event" << std::endl;
            return false;
        }
    }
    
    monitoring_ = true;
    samples_.clear();
    return true;
}

bool PerformanceMonitor::stopMonitoring() {
    if (!monitoring_) {
        return true;
    }
    
    for (int fd : perf_fds_) {
        if (ioctl(fd, PERF_EVENT_IOC_DISABLE, 0) == -1) {
            std::cerr << "Failed to disable perf event" << std::endl;
        }
    }
    
    monitoring_ = false;
    return true;
}

PerformanceSample PerformanceMonitor::readCounters() {
    PerformanceSample sample;
    sample.timestamp_ns = static_cast<uint64_t>(__builtin_ia32_rdtsc());
    
    if (perf_fds_.size() >= 6) {
        // Read individual counters
        uint64_t values[2];  // for read_format
        
        if (read(perf_fds_[0], values, sizeof(values)) == sizeof(values)) {
            sample.instructions = values[0];
        }
        
        if (read(perf_fds_[1], values, sizeof(values)) == sizeof(values)) {
            sample.cycles = values[0];
        }
        
        if (read(perf_fds_[2], values, sizeof(values)) == sizeof(values)) {
            sample.l1_misses = values[0];
        }
        
        if (read(perf_fds_[3], values, sizeof(values)) == sizeof(values)) {
            sample.l2_misses = values[0];
        }
        
        if (read(perf_fds_[4], values, sizeof(values)) == sizeof(values)) {
            sample.l3_misses = values[0];
        }
        
        if (read(perf_fds_[5], values, sizeof(values)) == sizeof(values)) {
            sample.branch_misses = values[0];
        }
    }
    
    // Classify phase based on current metrics (use header-only trained classifier)
    std::vector<double> features = {
        sample.l3_miss_rate(),
        sample.ipc(),
        sample.branch_miss_rate(),
        static_cast<double>(sample.l1_misses),
        static_cast<double>(sample.l2_misses),
        static_cast<double>(sample.instructions),
        static_cast<double>(sample.cycles)
    };
    sample.phase = TrainedPhaseClassifier::classify(features);
    
    return sample;
}

ExecutionPhase PerformanceMonitor::getCurrentPhase() const {
    if (samples_.empty()) {
        return ExecutionPhase::Unknown;
    }
    
    return samples_.back().phase;
}

std::vector<double> PerformanceMonitor::extractFeatures() const {
    if (samples_.empty()) {
        return {};
    }
    
    // Use the most recent sample for classification
    const auto& sample = samples_.back();
    
    return {
        sample.l3_miss_rate(),
        sample.ipc(), 
        sample.branch_miss_rate(),
        static_cast<double>(sample.l1_misses),
        static_cast<double>(sample.l2_misses)
    };
}



} // namespace morpheus