#include "../src/engine/graph/csr_graph.h"
#include "../src/engine/algorithms/bfs.h"
#include <iostream>
#include <cassert>

using namespace morpheus;

void test_bfs_small() {
    std::cout << "Testing BFS on small graph..." << std::endl;
    
    std::vector<std::pair<uint32_t, uint32_t>> edges = {
        {0, 1}, {1, 2}, {2, 3}, {3, 4}, {0, 5}
    };
    
    CSRGraph graph;
    graph.buildFromEdges(6, edges);
    
    BFS bfs(graph);
    BFSResult result = bfs.run(0);
    
    // Check distances
    assert(result.distances[0] == 0);
    assert(result.distances[1] == 1);
    assert(result.distances[2] == 2);
    assert(result.distances[3] == 3);
    assert(result.distances[4] == 4);
    assert(result.distances[5] == 1);
    
    // Check parents
    assert(result.parents[0] == 0);
    assert(result.parents[1] == 0);
    assert(result.parents[2] == 1);
    assert(result.parents[5] == 0);
    
    // Validate result
    assert(BFS::validate(graph, result, 0));
    
    std::cout << "BFS small graph test passed" << std::endl;
}

void test_bfs_disconnected() {
    std::cout << "Testing BFS on disconnected graph..." << std::endl;
    
    std::vector<std::pair<uint32_t, uint32_t>> edges = {
        {0, 1}, {1, 2}, {3, 4}, {4, 5}  // Two disconnected components
    };
    
    CSRGraph graph;
    graph.buildFromEdges(6, edges);
    
    BFS bfs(graph);
    BFSResult result = bfs.run(0);
    
    // Check that vertices in second component are unreachable
    assert(result.distances[3] == -1);
    assert(result.distances[4] == -1);
    assert(result.distances[5] == -1);
    
    // Validate result
    assert(BFS::validate(graph, result, 0));
    
    std::cout << "BFS disconnected graph test passed" << std::endl;
}

void test_bfs_cyclic() {
    std::cout << "Testing BFS on cyclic graph..." << std::endl;
    
    std::vector<std::pair<uint32_t, uint32_t>> edges = {
        {0, 1}, {1, 2}, {2, 0}, {1, 3}, {3, 4}
    };
    
    CSRGraph graph;
    graph.buildFromEdges(5, edges);
    
    BFS bfs(graph);
    BFSResult result = bfs.run(0);
    
    // Check distances - should find shortest paths despite cycle
    assert(result.distances[0] == 0);
    assert(result.distances[1] == 1);
    assert(result.distances[2] == 2);  // Through 0->1->2, not 0->1->2->0
    assert(result.distances[3] == 2);
    assert(result.distances[4] == 3);
    
    // Validate result
    assert(BFS::validate(graph, result, 0));
    
    std::cout << "BFS cyclic graph test passed" << std::endl;
}

void test_bfs_multi_source() {
    std::cout << "Testing multi-source BFS..." << std::endl;
    
    std::vector<std::pair<uint32_t, uint32_t>> edges = {
        {0, 2}, {1, 2}, {2, 3}, {3, 4}
    };
    
    CSRGraph graph;
    graph.buildFromEdges(5, edges);
    
    BFS bfs(graph);
    std::vector<uint32_t> sources = {0, 1};
    BFSResult result = bfs.runMultiSource(sources);
    
    // Check distances from nearest source
    assert(result.distances[0] == 0);
    assert(result.distances[1] == 0);
    assert(result.distances[2] == 1);  // Reachable from both sources in 1 step
    assert(result.distances[3] == 2);
    assert(result.distances[4] == 3);
    
    std::cout << "Multi-source BFS test passed" << std::endl;
}

void test_bfs_early_termination() {
    std::cout << "Testing BFS with early termination..." << std::endl;
    
    std::vector<std::pair<uint32_t, uint32_t>> edges;
    // Create a chain 0->1->2->3->4->5
    for (uint32_t i = 0; i < 5; ++i) {
        edges.emplace_back(i, i + 1);
    }
    
    CSRGraph graph;
    graph.buildFromEdges(6, edges);
    
    BFS bfs(graph);
    BFSResult result = bfs.runUntil(0, 2);  // Stop at depth 2
    
    // Check that we only reached depth 2
    assert(result.distances[0] == 0);
    assert(result.distances[1] == 1);
    assert(result.distances[2] == 2);
    assert(result.distances[3] == -1);  // Not reached due to early termination
    assert(result.distances[4] == -1);
    assert(result.distances[5] == -1);
    
    std::cout << "BFS early termination test passed" << std::endl;
}

int main() {
    std::cout << "Starting BFS tests..." << std::endl;
    
    try {
        test_bfs_small();
        test_bfs_disconnected();
        test_bfs_cyclic();
        test_bfs_multi_source();
        test_bfs_early_termination();
        
        std::cout << "All BFS tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}