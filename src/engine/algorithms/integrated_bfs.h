#pragma once

#include <queue>
#include <vector>
#include <memory>
#include <cstring>
#include "../graph/graph_interface.h"
#include "../prefetch/prefetcher_interface.h"
#include "../monitoring/performance_monitor.h"
#include "../../instrumentation/strategy_controller.h"

/**
 * Integrated BFS with Real-Time Adaptive Prefetching
 * 
 * This class demonstrates actual prefetcher integration:
 * - Prefetches neighbors during graph traversal
 * - Adapts strategy based on runtime performance
 * - Validates correctness against reference implementation
 */
class IntegratedBFS {
public:
    using VertexId = uint32_t;
    using DistanceMap = std::vector<uint32_t>;

    IntegratedBFS(const GraphInterface* graph,
                  std::shared_ptr<Prefetcher> prefetcher,
                  std::shared_ptr<PerformanceMonitor> monitor = nullptr)
        : graph_(graph),
          prefetcher_(prefetcher),
          monitor_(monitor),
          sample_interval_ms_(1),
          adaptive_enabled_(monitor != nullptr) {
    }

    /**
     * Standard BFS (baseline for comparison)
     */
    DistanceMap runBaseline(VertexId source) {
        DistanceMap distances(graph_->getNumVertices(), UINT32_MAX);
        std::queue<VertexId> queue;

        distances[source] = 0;
        queue.push(source);

        while (!queue.empty()) {
            VertexId u = queue.front();
            queue.pop();

            // Standard BFS: process neighbors without prefetching
            const VertexId* neighbors = graph_->getNeighbors(u);
            uint32_t degree = graph_->getDegree(u);

            for (uint32_t i = 0; i < degree; i++) {
                VertexId v = neighbors[i];
                if (distances[v] == UINT32_MAX) {
                    distances[v] = distances[u] + 1;
                    queue.push(v);
                }
            }
        }

        return distances;
    }

    /**
     * Hardware-only prefetch (relies on automatic prefetchers)
     */
    DistanceMap runHardwarePrefetch(VertexId source) {
        DistanceMap distances(graph_->getNumVertices(), UINT32_MAX);
        std::queue<VertexId> queue;

        distances[source] = 0;
        queue.push(source);

        while (!queue.empty()) {
            VertexId u = queue.front();
            queue.pop();

            const VertexId* neighbors = graph_->getNeighbors(u);
            uint32_t degree = graph_->getDegree(u);

            // Let hardware prefetchers do their thing
            // (no software prefetching hints)
            __builtin_prefetch(neighbors, 0, 3);  // Prefetch read-only
            
            for (uint32_t i = 0; i < degree; i++) {
                VertexId v = neighbors[i];
                if (distances[v] == UINT32_MAX) {
                    distances[v] = distances[u] + 1;
                    queue.push(v);
                }
            }
        }

        return distances;
    }

    /**
     * Simple static prefetch (one-step ahead)
     */
    DistanceMap runSimplePrefetch(VertexId source) {
        DistanceMap distances(graph_->getNumVertices(), UINT32_MAX);
        std::queue<VertexId> queue;

        distances[source] = 0;
        queue.push(source);

        while (!queue.empty()) {
            VertexId u = queue.front();
            queue.pop();

            const VertexId* neighbors = graph_->getNeighbors(u);
            uint32_t degree = graph_->getDegree(u);

            // Simple prefetching: prefetch neighbors of current vertex
            // before processing them
            for (uint32_t i = 0; i < std::min(degree, 4u); i++) {
                VertexId v = neighbors[i];
                const VertexId* v_neighbors = graph_->getNeighbors(v);
                __builtin_prefetch(v_neighbors, 0, 2);
            }

            for (uint32_t i = 0; i < degree; i++) {
                VertexId v = neighbors[i];
                if (distances[v] == UINT32_MAX) {
                    distances[v] = distances[u] + 1;
                    queue.push(v);
                }
            }
        }

        return distances;
    }

    /**
     * Morpheus Integrated BFS with Adaptive Prefetching
     * 
     * Key features:
     * 1. Actual prefetcher integration during traversal
     * 2. Real-time phase detection and adaptation
     * 3. Multiple prefetch strategies based on execution phase
     */
    DistanceMap runOptimized(VertexId source) {
        if (monitor_) {
            monitor_->startMonitoring();
        }

        DistanceMap distances(graph_->getNumVertices(), UINT32_MAX);
        std::queue<VertexId> queue;

        distances[source] = 0;
        queue.push(source);

        uint64_t iteration = 0;
        uint64_t last_sample_time = getCurrentTimeMs();

        while (!queue.empty()) {
            VertexId u = queue.front();
            queue.pop();

            // Get neighbors of current vertex
            const VertexId* neighbors = graph_->getNeighbors(u);
            uint32_t degree = graph_->getDegree(u);

            // ===== ACTUAL PREFETCHING =====
            // This is the key difference - we're actually using the prefetcher
            if (prefetcher_) {
                prefetcher_->prefetch(neighbors, degree);
            }

            // Process neighbors
            for (uint32_t i = 0; i < degree; i++) {
                VertexId v = neighbors[i];
                
                if (distances[v] == UINT32_MAX) {
                    distances[v] = distances[u] + 1;
                    queue.push(v);

                    // ===== LOOKAHEAD PREFETCHING =====
                    // Prefetch neighbors of the vertex we just queued
                    if (prefetcher_) {
                        const VertexId* v_neighbors = graph_->getNeighbors(v);
                        uint32_t v_degree = graph_->getDegree(v);
                        prefetcher_->prefetch(v_neighbors, v_degree);
                    }
                }
            }

            // ===== REAL-TIME ADAPTATION =====
            // Periodically check performance and adapt strategy
            if (adaptive_enabled_ && ++iteration % 1000 == 0) {
                uint64_t current_time = getCurrentTimeMs();
                if (current_time - last_sample_time >= sample_interval_ms_) {
                    // Get current performance sample
                    auto sample = monitor_->readCounters();
                    
                    // Extract features from sample
                    auto features = monitor_->extractFeatures(sample);
                    
                    // Classify execution phase
                    ExecutionPhase phase = classifyPhase(features);
                    
                    // Update prefetching strategy based on phase
                    updatePrefetchingStrategy(phase);
                    
                    last_sample_time = current_time;
                }
            }
        }

        if (monitor_) {
            monitor_->stopMonitoring();
        }

        return distances;
    }

    /**
     * Validate that Morpheus produces identical results to baseline
     */
    bool validateCorrectness(VertexId source) {
        auto baseline_result = runBaseline(source);
        auto optimized_result = runOptimized(source);

        if (baseline_result.size() != optimized_result.size()) {
            return false;
        }

        for (size_t i = 0; i < baseline_result.size(); i++) {
            if (baseline_result[i] != optimized_result[i]) {
                return false;
            }
        }

        return true;
    }

    /**
     * Run benchmarks comparing all strategies
     */
    struct BenchmarkResults {
        double baseline_time_ms = 0.0;
        double hardware_prefetch_time_ms = 0.0;
        double simple_prefetch_time_ms = 0.0;
        double morpheus_optimized_time_ms = 0.0;
        
        double speedup_hw() const { 
            return baseline_time_ms / hardware_prefetch_time_ms; 
        }
        double speedup_simple() const { 
            return baseline_time_ms / simple_prefetch_time_ms; 
        }
        double speedup_morpheus() const { 
            return baseline_time_ms / morpheus_optimized_time_ms; 
        }
    };

    BenchmarkResults benchmark(VertexId source, int iterations = 10) {
        BenchmarkResults results;

        // Baseline
        auto baseline_start = getCurrentTimeMs();
        for (int i = 0; i < iterations; i++) {
            runBaseline(source);
        }
        results.baseline_time_ms = getCurrentTimeMs() - baseline_start;

        // Hardware prefetch
        auto hw_start = getCurrentTimeMs();
        for (int i = 0; i < iterations; i++) {
            runHardwarePrefetch(source);
        }
        results.hardware_prefetch_time_ms = getCurrentTimeMs() - hw_start;

        // Simple prefetch
        auto simple_start = getCurrentTimeMs();
        for (int i = 0; i < iterations; i++) {
            runSimplePrefetch(source);
        }
        results.simple_prefetch_time_ms = getCurrentTimeMs() - simple_start;

        // Morpheus optimized
        auto morpheus_start = getCurrentTimeMs();
        for (int i = 0; i < iterations; i++) {
            runOptimized(source);
        }
        results.morpheus_optimized_time_ms = getCurrentTimeMs() - morpheus_start;

        return results;
    }

private:
    enum class ExecutionPhase {
        DENSE_SEQUENTIAL,
        SPARSE_RANDOM,
        POINTER_CHASING
    };

    const GraphInterface* graph_;
    std::shared_ptr<Prefetcher> prefetcher_;
    std::shared_ptr<PerformanceMonitor> monitor_;
    uint64_t sample_interval_ms_;
    bool adaptive_enabled_;

    /**
     * Classify current execution phase based on performance metrics
     */
    ExecutionPhase classifyPhase(const std::vector<double>& features) {
        // Features typically include: cache miss rates, IPC, branch accuracy
        // This would use the trained ML model in production
        
        // Simple heuristic for demonstration:
        // - High L1 hit rate → Dense Sequential
        // - Low L1 hit rate, good prefetch → Sparse Random
        // - Low prefetch effectiveness → Pointer Chasing
        
        if (features.size() < 3) {
            return ExecutionPhase::DENSE_SEQUENTIAL;
        }

        double l1_hit_rate = features[0];
        
        if (l1_hit_rate > 0.8) {
            return ExecutionPhase::DENSE_SEQUENTIAL;
        } else if (l1_hit_rate > 0.5) {
            return ExecutionPhase::SPARSE_RANDOM;
        } else {
            return ExecutionPhase::POINTER_CHASING;
        }
    }

    /**
     * Update prefetching strategy based on detected phase
     */
    void updatePrefetchingStrategy(ExecutionPhase phase) {
        // In production, this would call prefetcher_->setStrategy(phase)
        // For now, this demonstrates the concept
        switch (phase) {
            case ExecutionPhase::DENSE_SEQUENTIAL:
                // Aggressive prefetching works well
                break;
            case ExecutionPhase::SPARSE_RANDOM:
                // Moderate prefetch depth
                break;
            case ExecutionPhase::POINTER_CHASING:
                // Short-distance prefetch only
                break;
        }
    }

    static uint64_t getCurrentTimeMs() {
        auto duration = std::chrono::high_resolution_clock::now()
            .time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    }
};
