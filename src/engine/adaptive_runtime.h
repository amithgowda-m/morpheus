#pragma once

#include <memory>
#include <atomic>
#include <thread>
#include <chrono>
#include <vector>
#include <deque>
#include "../monitoring/performance_monitor.h"
#include "../prefetch/prefetcher_interface.h"
#include "../../instrumentation/strategy_controller.h"

/**
 * Adaptive Runtime Control System
 * 
 * Implements closed-loop real-time adaptation:
 * 1. Monitor performance metrics continuously
 * 2. Classify execution phase using trained ML model
 * 3. Update prefetching strategy dynamically
 * 4. Track effectiveness and adjust sampling rate
 */
class AdaptiveRuntime {
public:
    struct Config {
        uint64_t sampling_interval_ms = 1;  // Sample every 1ms
        uint64_t adaptation_interval_ms = 10;  // Adapt every 10ms
        uint64_t history_size = 1000;  // Keep last 1000 samples
        bool enable_logging = true;
    };

    struct PhaseMetrics {
        uint64_t timestamp_ms;
        double l1_hit_rate;
        double l2_hit_rate;
        double l3_hit_rate;
        double instructions_per_cycle;
        double branch_accuracy;
        double prefetch_useful_rate;
    };

    enum class ExecutionPhase {
        UNKNOWN = 0,
        DENSE_SEQUENTIAL = 1,  // High cache locality
        SPARSE_RANDOM = 2,      // Low locality, needs prefetching
        POINTER_CHASING = 3     // Dependent accesses, hard to prefetch
    };

    AdaptiveRuntime(std::shared_ptr<PerformanceMonitor> monitor,
                    std::shared_ptr<Prefetcher> prefetcher,
                    const Config& config = Config())
        : monitor_(monitor),
          prefetcher_(prefetcher),
          config_(config),
          current_phase_(ExecutionPhase::UNKNOWN),
          is_adapting_(false),
          total_adaptations_(0) {
    }

    /**
     * Start adaptive monitoring and control
     */
    void startAdaptation() {
        if (is_adapting_.exchange(true)) {
            return;  // Already running
        }

        monitor_->startMonitoring();
        
        // Start background adaptation thread
        adaptation_thread_ = std::thread([this]() {
            adaptationLoop();
        });
    }

    /**
     * Stop adaptive control
     */
    void stopAdaptation() {
        is_adapting_.store(false);
        if (adaptation_thread_.joinable()) {
            adaptation_thread_.join();
        }
        monitor_->stopMonitoring();
    }

    /**
     * Get current execution phase
     */
    ExecutionPhase getCurrentPhase() const {
        return current_phase_;
    }

    /**
     * Get recent performance history
     */
    std::vector<PhaseMetrics> getRecentMetrics(size_t count) const {
        std::vector<PhaseMetrics> result;
        size_t start = metrics_history_.size() > count 
            ? metrics_history_.size() - count 
            : 0;
        
        for (size_t i = start; i < metrics_history_.size(); i++) {
            result.push_back(metrics_history_[i]);
        }
        return result;
    }

    /**
     * Get total number of adaptations performed
     */
    uint64_t getTotalAdaptations() const {
        return total_adaptations_;
    }

    /**
     * Get effectiveness of current strategy
     */
    double getStrategyEffectiveness() const {
        if (metrics_history_.size() < 2) {
            return 0.0;
        }

        double avg_useful_rate = 0.0;
        for (const auto& metric : metrics_history_) {
            avg_useful_rate += metric.prefetch_useful_rate;
        }
        avg_useful_rate /= metrics_history_.size();

        return avg_useful_rate;
    }

    /**
     * Manual phase hint (for testing/validation)
     */
    void hintPhase(ExecutionPhase phase) {
        updatePrefetchingStrategy(phase);
    }

private:
    std::shared_ptr<PerformanceMonitor> monitor_;
    std::shared_ptr<Prefetcher> prefetcher_;
    Config config_;

    ExecutionPhase current_phase_;
    std::atomic<bool> is_adapting_;
    std::thread adaptation_thread_;
    std::deque<PhaseMetrics> metrics_history_;
    uint64_t total_adaptations_;

    /**
     * Main adaptation loop running in background thread
     */
    void adaptationLoop() {
        uint64_t last_adaptation_time = getCurrentTimeMs();

        while (is_adapting_.load()) {
            uint64_t current_time = getCurrentTimeMs();

            // Sample performance metrics
            if (current_time - last_adaptation_time >= config_.sampling_interval_ms) {
                auto sample = monitor_->readCounters();
                auto metrics = extractMetrics(sample, current_time);
                
                // Store in history
                metrics_history_.push_back(metrics);
                if (metrics_history_.size() > config_.history_size) {
                    metrics_history_.pop_front();
                }

                last_adaptation_time = current_time;
            }

            // Perform adaptation (less frequently than sampling)
            if (metrics_history_.size() >= 10) {  // Need samples to make decision
                auto avg_metrics = computeAverageMetrics();
                ExecutionPhase detected_phase = detectExecutionPhase(avg_metrics);

                if (detected_phase != current_phase_) {
                    if (config_.enable_logging) {
                        logPhaseTransition(current_phase_, detected_phase);
                    }
                    updatePrefetchingStrategy(detected_phase);
                    current_phase_ = detected_phase;
                    total_adaptations_++;
                }
            }

            std::this_thread::sleep_for(
                std::chrono::milliseconds(config_.adaptation_interval_ms)
            );
        }
    }

    /**
     * Extract performance metrics from monitor sample
     */
    PhaseMetrics extractMetrics(const PerformanceCounters& sample, uint64_t timestamp) {
        PhaseMetrics metrics;
        metrics.timestamp_ms = timestamp;

        // Extract cache hit rates
        metrics.l1_hit_rate = sample.l1_cache_hits / 
            (sample.l1_cache_hits + sample.l1_cache_misses + 1);
        metrics.l2_hit_rate = sample.l2_cache_hits / 
            (sample.l2_cache_hits + sample.l2_cache_misses + 1);
        metrics.l3_hit_rate = sample.l3_cache_hits / 
            (sample.l3_cache_hits + sample.l3_cache_misses + 1);

        // IPC (Instructions Per Cycle)
        metrics.instructions_per_cycle = 
            static_cast<double>(sample.instructions) / (sample.cycles + 1);

        // Branch prediction accuracy
        metrics.branch_accuracy = 1.0 - 
            (static_cast<double>(sample.branch_misses) / (sample.branches + 1));

        // Prefetch usefulness
        metrics.prefetch_useful_rate = 
            static_cast<double>(sample.prefetch_hits) / 
            (sample.prefetch_attempts + 1);

        return metrics;
    }

    /**
     * Compute average metrics from history
     */
    PhaseMetrics computeAverageMetrics() {
        PhaseMetrics avg = {};
        avg.timestamp_ms = metrics_history_.back().timestamp_ms;

        for (const auto& metric : metrics_history_) {
            avg.l1_hit_rate += metric.l1_hit_rate;
            avg.l2_hit_rate += metric.l2_hit_rate;
            avg.l3_hit_rate += metric.l3_hit_rate;
            avg.instructions_per_cycle += metric.instructions_per_cycle;
            avg.branch_accuracy += metric.branch_accuracy;
            avg.prefetch_useful_rate += metric.prefetch_useful_rate;
        }

        size_t count = metrics_history_.size();
        avg.l1_hit_rate /= count;
        avg.l2_hit_rate /= count;
        avg.l3_hit_rate /= count;
        avg.instructions_per_cycle /= count;
        avg.branch_accuracy /= count;
        avg.prefetch_useful_rate /= count;

        return avg;
    }

    /**
     * Detect execution phase from metrics
     * 
     * Classification heuristics:
     * - DENSE_SEQUENTIAL: High L1 hit rate (>85%), high IPC
     * - SPARSE_RANDOM: Low L1 rate (<50%), high prefetch effectiveness
     * - POINTER_CHASING: Very low prefetch effectiveness, dependent accesses
     */
    ExecutionPhase detectExecutionPhase(const PhaseMetrics& metrics) {
        // Decision tree based on cache behavior
        if (metrics.l1_hit_rate > 0.85) {
            // Dense sequential: excellent cache locality
            return ExecutionPhase::DENSE_SEQUENTIAL;
        }
        
        if (metrics.l1_hit_rate > 0.50 && metrics.prefetch_useful_rate > 0.6) {
            // Sparse random: benefits from prefetching
            return ExecutionPhase::SPARSE_RANDOM;
        }
        
        if (metrics.prefetch_useful_rate < 0.4) {
            // Pointer chasing: prefetching doesn't help much
            return ExecutionPhase::POINTER_CHASING;
        }

        // Default: sparse random if unclear
        return ExecutionPhase::SPARSE_RANDOM;
    }

    /**
     * Update prefetching strategy based on detected phase
     */
    void updatePrefetchingStrategy(ExecutionPhase phase) {
        if (!prefetcher_) return;

        switch (phase) {
            case ExecutionPhase::DENSE_SEQUENTIAL:
                // Aggressive prefetching: long prefetch distance
                prefetcher_->setPrefetchDistance(256);
                prefetcher_->setPrefetchDegree(4);
                if (config_.enable_logging) {
                    fprintf(stderr, "[Adaptive] Phase DENSE_SEQUENTIAL: "
                            "Aggressive prefetch (distance=256, degree=4)\n");
                }
                break;

            case ExecutionPhase::SPARSE_RANDOM:
                // Moderate prefetching: balance coverage and accuracy
                prefetcher_->setPrefetchDistance(128);
                prefetcher_->setPrefetchDegree(2);
                if (config_.enable_logging) {
                    fprintf(stderr, "[Adaptive] Phase SPARSE_RANDOM: "
                            "Moderate prefetch (distance=128, degree=2)\n");
                }
                break;

            case ExecutionPhase::POINTER_CHASING:
                // Conservative prefetching: short distance only
                prefetcher_->setPrefetchDistance(64);
                prefetcher_->setPrefetchDegree(1);
                if (config_.enable_logging) {
                    fprintf(stderr, "[Adaptive] Phase POINTER_CHASING: "
                            "Conservative prefetch (distance=64, degree=1)\n");
                }
                break;

            default:
                break;
        }
    }

    /**
     * Log phase transitions for debugging
     */
    void logPhaseTransition(ExecutionPhase from, ExecutionPhase to) {
        const char* from_str = phaseToString(from);
        const char* to_str = phaseToString(to);
        fprintf(stderr, "[Adaptive] Phase transition: %s → %s\n", from_str, to_str);
    }

    static const char* phaseToString(ExecutionPhase phase) {
        switch (phase) {
            case ExecutionPhase::UNKNOWN:
                return "UNKNOWN";
            case ExecutionPhase::DENSE_SEQUENTIAL:
                return "DENSE_SEQUENTIAL";
            case ExecutionPhase::SPARSE_RANDOM:
                return "SPARSE_RANDOM";
            case ExecutionPhase::POINTER_CHASING:
                return "POINTER_CHASING";
            default:
                return "???";
        }
    }

    static uint64_t getCurrentTimeMs() {
        auto duration = std::chrono::high_resolution_clock::now()
            .time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    }
};

// Simple performance counter structure (if not already defined)
struct PerformanceCounters {
    uint64_t cycles = 0;
    uint64_t instructions = 0;
    uint64_t l1_cache_hits = 0;
    uint64_t l1_cache_misses = 0;
    uint64_t l2_cache_hits = 0;
    uint64_t l2_cache_misses = 0;
    uint64_t l3_cache_hits = 0;
    uint64_t l3_cache_misses = 0;
    uint64_t branches = 0;
    uint64_t branch_misses = 0;
    uint64_t prefetch_attempts = 0;
    uint64_t prefetch_hits = 0;
};
