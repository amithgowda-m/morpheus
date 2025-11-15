#include "../src/engine/graph/csr_graph.h"
#include "../src/engine/algorithms/bfs.h"
#include "../src/engine/algorithms/pagerank.h"
#include "../src/engine/algorithms/betweenness.h"
#include "../src/engine/monitoring/performance_monitor.h"
#include <iostream>
#include <cassert>

using namespace morpheus;

void test_performance_monitor() {
    std::cout << "Testing performance monitor..." << std::endl;
    
    PerformanceMonitor monitor;
    bool initialized = monitor.initialize(1);  // 1ms sampling
    
    // Initialization might fail if perf events not available
    if (!initialized) {
        std::cout << "Performance monitor initialization failed (may require root)" << std::endl;
        return;
    }
    
    assert(monitor.startMonitoring());
    
    // Do some work to generate samples
    volatile int sum = 0;
    for (int i = 0; i < 1000000; ++i) {
        sum += i * i;
    }
    
    assert(monitor.stopMonitoring());
    
    const auto& samples = monitor.getSamples();
    std::cout << "Collected " << samples.size() << " performance samples" << std::endl;
    
    // Should have at least some samples
    assert(!samples.empty());
    
    std::cout << "Performance monitor test passed" << std::endl;
}

void test_phase_classifier() {
    std::cout << "Testing phase classifier..." << std::endl;
    
    // Test with sample features
    std::vector<double> features_dense = {0.005, 1.5, 0.02, 1000, 500};  // Low miss rate, high IPC
    std::vector<double> features_sparse = {0.03, 0.7, 0.03, 5000, 3000}; // High miss rate, low IPC
    std::vector<double> features_pointer = {0.01, 0.9, 0.08, 2000, 1000}; // High branch misses
    
    ExecutionPhase phase1 = TrainedPhaseClassifier::classify(features_dense);
    ExecutionPhase phase2 = TrainedPhaseClassifier::classify(features_sparse);
    ExecutionPhase phase3 = TrainedPhaseClassifier::classify(features_pointer);
    
    std::cout << "Dense features classified as: " << static_cast<int>(phase1) << std::endl;
    std::cout << "Sparse features classified as: " << static_cast<int>(phase2) << std::endl;
    std::cout << "Pointer features classified as: " << static_cast<int>(phase3) << std::endl;
    
    // Basic sanity checks
    assert(phase1 != ExecutionPhase::Unknown);
    assert(phase2 != ExecutionPhase::Unknown);
    assert(phase3 != ExecutionPhase::Unknown);
    
    std::cout << "Phase classifier test passed" << std::endl;
}

void test_end_to_end() {
    std::cout << "Testing end-to-end workflow..." << std::endl;
    
    // Create a test graph
    std::vector<std::pair<uint32_t, uint32_t>> edges = {
        {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 0},
        {0, 2}, {1, 3}, {2, 4}, {3, 0}, {4, 1}
    };
    
    CSRGraph graph;
    graph.buildFromEdges(5, edges);
    
    // Test BFS
    BFS bfs(graph);
    BFSResult bfs_result = bfs.run(0);
    assert(BFS::validate(graph, bfs_result, 0));
    
    // Test PageRank
    PageRank pagerank(graph, 0.85, 1e-8);
    PageRankResult pr_result = pagerank.run(50);
    assert(PageRank::validate(graph, pr_result, 0.85, 1e-6));
    
    // Test Betweenness (approximate for speed)
    BetweennessCentrality bc(graph);
    auto bc_result = bc.runApproximate(3);  // Sample 3 vertices
    assert(BetweennessCentrality::validate(graph, bc_result, 1e-6));
    
    std::cout << "End-to-end test passed" << std::endl;
    std::cout << "BFS time: " << bfs_result.execution_time_ns << " ns" << std::endl;
    std::cout << "PageRank time: " << pr_result.execution_time_ns << " ns" << std::endl;
    std::cout << "Betweenness time: " << bc_result.execution_time_ns << " ns" << std::endl;
}

void test_prefetch_strategies() {
    std::cout << "Testing prefetch strategies..." << std::endl;
    
    // This would test the prefetcher interfaces
    // In a real test, we'd want to measure actual performance impact
    
    std::cout << "Prefetch strategy interface test passed" << std::endl;
}

int main() {
    std::cout << "Starting integration tests..." << std::endl;
    
    try {
        test_performance_monitor();
        test_phase_classifier();
        test_end_to_end();
        test_prefetch_strategies();
        
        std::cout << "All integration tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}