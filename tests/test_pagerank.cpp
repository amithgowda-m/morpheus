#include "../src/engine/graph/csr_graph.h"
#include "../src/engine/algorithms/pagerank.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace morpheus;

void test_pagerank_small() {
    std::cout << "Testing PageRank on small graph..." << std::endl;
    
    std::vector<std::pair<uint32_t, uint32_t>> edges = {
        {0, 1}, {1, 2}, {2, 0}, {1, 3}
    };
    
    CSRGraph graph;
    graph.buildFromEdges(4, edges);
    
    PageRank pagerank(graph, 0.85, 1e-8);
    PageRankResult result = pagerank.run(20);
    
    // Check basic properties
    assert(result.iterations > 0);
    assert(result.iterations <= 20);
    assert(result.final_residual < 1e-8);
    
    // Check that scores sum to approximately 1
    double sum = 0.0;
    for (double score : result.scores) {
        sum += score;
        assert(score >= 0.0);
    }
    assert(std::abs(sum - 1.0) < 1e-6);
    
    // Validate result
    assert(PageRank::validate(graph, result, 0.85, 1e-6));
    
    std::cout << "PageRank small graph test passed" << std::endl;
    std::cout << "Converged in " << result.iterations << " iterations" << std::endl;
}

void test_pagerank_dangling() {
    std::cout << "Testing PageRank with dangling node..." << std::endl;
    
    std::vector<std::pair<uint32_t, uint32_t>> edges = {
        {0, 1}, {1, 2}, {2, 0}  // Node 3 has no outgoing edges
    };
    
    CSRGraph graph;
    graph.buildFromEdges(4, edges);
    
    PageRank pagerank(graph, 0.85, 1e-8);
    PageRankResult result = pagerank.run(50);
    
    // Check that scores sum to 1
    double sum = 0.0;
    for (double score : result.scores) {
        sum += score;
    }
    assert(std::abs(sum - 1.0) < 1e-6);
    
    // Dangling node should have some rank
    assert(result.scores[3] > 0.0);
    
    // Validate result
    assert(PageRank::validate(graph, result, 0.85, 1e-6));
    
    std::cout << "PageRank dangling node test passed" << std::endl;
}

void test_pagerank_personalized() {
    std::cout << "Testing personalized PageRank..." << std::endl;
    
    std::vector<std::pair<uint32_t, uint32_t>> edges = {
        {0, 1}, {1, 2}, {2, 0}
    };
    
    CSRGraph graph;
    graph.buildFromEdges(3, edges);
    
    std::vector<double> personalization = {0.5, 0.3, 0.2};
    
    PageRank pagerank(graph, 0.85, 1e-8);
    PageRankResult result = pagerank.runPersonalized(personalization, 50);
    
    // Check convergence
    assert(result.iterations > 0);
    assert(result.final_residual < 1e-8);
    
    // Check that scores sum to 1
    double sum = 0.0;
    for (double score : result.scores) {
        sum += score;
        assert(score >= 0.0);
    }
    assert(std::abs(sum - 1.0) < 1e-6);
    
    // With high personalization on node 0, it should have higher rank
    assert(result.scores[0] > result.scores[2]);
    
    std::cout << "Personalized PageRank test passed" << std::endl;
}

void test_pagerank_convergence() {
    std::cout << "Testing PageRank convergence..." << std::endl;
    
    // Create a larger graph for convergence test
    std::vector<std::pair<uint32_t, uint32_t>> edges;
    const uint32_t n = 100;
    
    // Create a random-like graph
    for (uint32_t i = 0; i < n; ++i) {
        for (uint32_t j = 0; j < 3; ++j) {
            uint32_t dest = (i + j * 7) % n;  // Simple deterministic pattern
            if (dest != i) {
                edges.emplace_back(i, dest);
            }
        }
    }
    
    CSRGraph graph;
    graph.buildFromEdges(n, edges);
    
    PageRank pagerank(graph, 0.85, 1e-10);  // Stricter tolerance
    PageRankResult result = pagerank.run(1000);
    
    // Should converge within reasonable iterations
    assert(result.iterations < 1000);
    assert(result.final_residual < 1e-10);
    
    // Check validity
    assert(PageRank::validate(graph, result, 0.85, 1e-6));
    
    std::cout << "PageRank convergence test passed" << std::endl;
    std::cout << "Converged in " << result.iterations << " iterations" << std::endl;
}

void test_pagerank_star_graph() {
    std::cout << "Testing PageRank on star graph..." << std::endl;
    
    std::vector<std::pair<uint32_t, uint32_t>> edges;
    const uint32_t n = 5;
    
    // Star graph: center is node 0
    for (uint32_t i = 1; i < n; ++i) {
        edges.emplace_back(0, i);
        edges.emplace_back(i, 0);
    }
    
    CSRGraph graph;
    graph.buildFromEdges(n, edges);
    
    PageRank pagerank(graph, 0.85, 1e-8);
    PageRankResult result = pagerank.run(50);
    
    // Center should have higher rank than leaves
    assert(result.scores[0] > result.scores[1]);
    
    // All leaves should have similar ranks
    for (uint32_t i = 2; i < n; ++i) {
        assert(std::abs(result.scores[1] - result.scores[i]) < 1e-6);
    }
    
    // Validate result
    assert(PageRank::validate(graph, result, 0.85, 1e-6));
    
    std::cout << "PageRank star graph test passed" << std::endl;
}

int main() {
    std::cout << "Starting PageRank tests..." << std::endl;
    
    try {
        test_pagerank_small();
        test_pagerank_dangling();
        test_pagerank_personalized();
        test_pagerank_convergence();
        test_pagerank_star_graph();
        
        std::cout << "All PageRank tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}