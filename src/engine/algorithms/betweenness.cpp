#include "betweenness.h"
#include "../../utils/timer.h"
#include <queue>
#include <stack>
#include <vector>
#include <random>
#include <algorithm>

namespace morpheus {

BetweennessCentrality::BetweennessCentrality(const CSRGraph& graph) : graph_(graph) {}

BetweennessResult BetweennessCentrality::runApproximate(uint32_t sample_size, bool normalized) {
    BetweennessResult result(graph_.numVertices());
    Timer timer;
    
    if (sample_size >= graph_.numVertices()) {
        sample_size = graph_.numVertices();
    }
    
    std::vector<uint32_t> vertices(graph_.numVertices());
    for (uint32_t i = 0; i < graph_.numVertices(); ++i) {
        vertices[i] = i;
    }
    
    // Simple random sampling
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(vertices.begin(), vertices.end(), g);
    
    for (uint32_t i = 0; i < sample_size; ++i) {
        uint32_t source = vertices[i];
        
        // Brandes algorithm for single source
        std::vector<std::vector<uint32_t>> predecessors(graph_.numVertices());
        std::vector<int32_t> distances(graph_.numVertices(), -1);
        std::vector<double> shortest_paths_count(graph_.numVertices(), 0.0);
        
        std::queue<uint32_t> queue;
        std::stack<uint32_t> stack;
        
        // Initialize for source
        distances[source] = 0;
        shortest_paths_count[source] = 1.0;
        queue.push(source);
        
        // BFS phase to find shortest paths
        while (!queue.empty()) {
            uint32_t u = queue.front();
            queue.pop();
            stack.push(u);
            
            uint32_t degree = graph_.getDegree(u);
            const uint32_t* neighbors = graph_.getNeighbors(u);
            
            for (uint32_t j = 0; j < degree; ++j) {
                uint32_t v = neighbors[j];
                
                if (distances[v] == -1) {
                    distances[v] = distances[u] + 1;
                    queue.push(v);
                }
                
                if (distances[v] == distances[u] + 1) {
                    shortest_paths_count[v] += shortest_paths_count[u];
                    predecessors[v].push_back(u);
                }
            }
        }
        
        // Accumulation phase - back propagation of dependencies
        std::vector<double> dependency(graph_.numVertices(), 0.0);
        
        while (!stack.empty()) {
            uint32_t w = stack.top();
            stack.pop();
            
            for (uint32_t v : predecessors[w]) {
                dependency[v] += (shortest_paths_count[v] / shortest_paths_count[w]) * (1.0 + dependency[w]);
            }
            
            if (w != source) {
                result.centrality[w] += dependency[w];
            }
        }
        
        result.shortest_paths_count++;
    }
    
    // Scale by sampling factor
    double scale_factor = static_cast<double>(graph_.numVertices()) / sample_size;
    for (uint32_t i = 0; i < graph_.numVertices(); ++i) {
        result.centrality[i] *= scale_factor;
    }
    
    // Normalize for undirected graphs
    if (normalized && graph_.numVertices() > 2) {
        double factor = 2.0 / ((graph_.numVertices() - 1) * (graph_.numVertices() - 2));
        for (uint32_t i = 0; i < graph_.numVertices(); ++i) {
            result.centrality[i] *= factor;
        }
    }
    
    result.execution_time_ns = timer.elapsedNanoseconds();
    return result;
}

bool BetweennessCentrality::validate(const CSRGraph& graph, const BetweennessResult& result, double tolerance) {
    // Check that all centrality values are non-negative
    for (double centrality : result.centrality) {
        if (centrality < -tolerance) {
            return false;
        }
    }
    
    // For connected graphs, some vertices should have positive betweenness
    bool has_positive = false;
    for (double centrality : result.centrality) {
        if (centrality > tolerance) {
            has_positive = true;
            break;
        }
    }
    
    if (!has_positive && graph.numVertices() > 2) {
        return false;
    }
    
    return true;
}

} // namespace morpheus