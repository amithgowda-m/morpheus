#include "../src/engine/graph/csr_graph.h"
#include "../src/engine/graph/graph_generator.h"
#include <iostream>
#include <cassert>

using namespace morpheus;

void test_empty_graph() {
    std::cout << "Testing empty graph..." << std::endl;
    
    CSRGraph graph;
    assert(graph.numVertices() == 0);
    assert(graph.numEdges() == 0);
    assert(graph.validate());
    
    std::cout << "Empty graph test passed" << std::endl;
}

void test_small_graph() {
    std::cout << "Testing small graph..." << std::endl;
    
    std::vector<std::pair<uint32_t, uint32_t>> edges = {
        {0, 1}, {1, 2}, {2, 0}, {1, 3}
    };
    
    CSRGraph graph;
    graph.buildFromEdges(4, edges);
    
    assert(graph.numVertices() == 4);
    assert(graph.numEdges() == 4);
    assert(graph.validate());
    
    // Check degrees
    assert(graph.getDegree(0) == 1);  // Only edge to 1
    assert(graph.getDegree(1) == 2);  // Edges to 2 and 3
    assert(graph.getDegree(2) == 1);  // Only edge to 0
    assert(graph.getDegree(3) == 0);  // No outgoing edges
    
    // Check neighbors
    const uint32_t* neighbors0 = graph.getNeighbors(0);
    assert(neighbors0 != nullptr);
    assert(neighbors0[0] == 1);
    
    const uint32_t* neighbors1 = graph.getNeighbors(1);
    assert(neighbors1 != nullptr);
    assert(neighbors1[0] == 2);
    assert(neighbors1[1] == 3);
    
    std::cout << "Small graph test passed" << std::endl;
}

void test_weighted_graph() {
    std::cout << "Testing weighted graph..." << std::endl;
    
    std::vector<std::tuple<uint32_t, uint32_t, float>> edges = {
        {0, 1, 1.5f}, {1, 2, 2.0f}, {2, 0, 0.5f}
    };
    
    CSRGraph graph;
    graph.buildFromWeightedEdges(3, edges);
    
    assert(graph.numVertices() == 3);
    assert(graph.numEdges() == 3);
    assert(graph.isWeighted());
    assert(graph.validate());
    
    // Check weights
    const float* weights0 = graph.getWeights(0);
    assert(weights0 != nullptr);
    assert(weights0[0] == 1.5f);
    
    const float* weights1 = graph.getWeights(1);
    assert(weights1 != nullptr);
    assert(weights1[0] == 2.0f);
    
    std::cout << "Weighted graph test passed" << std::endl;
}

void test_graph_generator() {
    std::cout << "Testing graph generator..." << std::endl;
    
    GraphGenerator generator(42);
    
    // Test RMAT generation
    CSRGraph rmat_graph = generator.generateRMAT(4, 2);  // 16 vertices, 32 edges
    assert(rmat_graph.numVertices() == 16);
    assert(rmat_graph.numEdges() == 32);
    assert(rmat_graph.validate());
    
    // Test grid generation
    CSRGraph grid_graph = generator.generateGrid(3, 3);  // 9 vertices
    assert(grid_graph.numVertices() == 9);
    assert(grid_graph.validate());
    
    // Test star generation
    CSRGraph star_graph = generator.generateStar(5);  // 5 vertices
    assert(star_graph.numVertices() == 5);
    assert(star_graph.validate());
    
    std::cout << "Graph generator test passed" << std::endl;
}

void test_memory_usage() {
    std::cout << "Testing memory usage..." << std::endl;
    
    std::vector<std::pair<uint32_t, uint32_t>> edges;
    for (uint32_t i = 0; i < 1000; ++i) {
        for (uint32_t j = 0; j < 10; ++j) {
            edges.emplace_back(i, (i + j) % 1000);
        }
    }
    
    CSRGraph graph;
    graph.buildFromEdges(1000, edges);
    
    size_t memory_usage = graph.getMemoryUsage();
    assert(memory_usage > 0);
    
    std::cout << "Memory usage: " << memory_usage << " bytes" << std::endl;
    std::cout << "Memory usage test passed" << std::endl;
}

int main() {
    std::cout << "Starting graph tests..." << std::endl;
    
    try {
        test_empty_graph();
        test_small_graph();
        test_weighted_graph();
        test_graph_generator();
        test_memory_usage();
        
        std::cout << "All graph tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}