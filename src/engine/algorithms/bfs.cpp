#include "bfs.h"
#include "../../utils/timer.h"
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>

namespace morpheus {

BFS::BFS(const CSRGraph& graph) : graph_(graph) {}

BFSResult BFS::run(uint32_t source_vertex) {
    if (source_vertex >= graph_.numVertices()) {
        throw std::invalid_argument("Source vertex out of range");
    }
    
    BFSResult result(graph_.numVertices());
    Timer timer;
    
    // Use level-synchronized BFS for better cache behavior
    bfsLevelSync(source_vertex, result);
    
    result.execution_time_ns = timer.elapsedNanoseconds();
    return result;
}

BFSResult BFS::runUntil(uint32_t source_vertex, uint32_t max_depth) {
    BFSResult result(graph_.numVertices());
    Timer timer;
    
    std::vector<uint32_t> current_frontier;
    std::vector<uint32_t> next_frontier;
    
    result.distances[source_vertex] = 0;
    result.parents[source_vertex] = source_vertex;
    current_frontier.push_back(source_vertex);
    result.visitation_order.push_back(source_vertex);
    
    uint32_t depth = 0;
    
    while (!current_frontier.empty() && depth < max_depth) {
        next_frontier.clear();
        
        for (uint32_t vertex : current_frontier) {
            uint32_t degree = graph_.getDegree(vertex);
            const uint32_t* neighbors = graph_.getNeighbors(vertex);
            
            for (uint32_t i = 0; i < degree; ++i) {
                uint32_t neighbor = neighbors[i];
                result.edges_visited++;
                
                if (result.distances[neighbor] == -1) {
                    result.distances[neighbor] = depth + 1;
                    result.parents[neighbor] = vertex;
                    result.visitation_order.push_back(neighbor);
                    next_frontier.push_back(neighbor);
                }
            }
        }
        
        current_frontier.swap(next_frontier);
        depth++;
    }
    
    result.execution_time_ns = timer.elapsedNanoseconds();
    return result;
}

BFSResult BFS::runMultiSource(const std::vector<uint32_t>& sources) {
    BFSResult result(graph_.numVertices());
    Timer timer;
    
    std::vector<uint32_t> current_frontier = sources;
    std::vector<uint32_t> next_frontier;
    
    // Initialize sources
    for (uint32_t source : sources) {
        if (source < graph_.numVertices()) {
            result.distances[source] = 0;
            result.parents[source] = source;
            result.visitation_order.push_back(source);
        }
    }
    
    uint32_t depth = 0;
    
    while (!current_frontier.empty()) {
        next_frontier.clear();
        
        for (uint32_t vertex : current_frontier) {
            uint32_t degree = graph_.getDegree(vertex);
            const uint32_t* neighbors = graph_.getNeighbors(vertex);
            
            for (uint32_t i = 0; i < degree; ++i) {
                uint32_t neighbor = neighbors[i];
                result.edges_visited++;
                
                if (result.distances[neighbor] == -1) {
                    result.distances[neighbor] = depth + 1;
                    result.parents[neighbor] = vertex;
                    result.visitation_order.push_back(neighbor);
                    next_frontier.push_back(neighbor);
                }
            }
        }
        
        current_frontier.swap(next_frontier);
        depth++;
    }
    
    result.execution_time_ns = timer.elapsedNanoseconds();
    return result;
}

void BFS::bfsLevelSync(uint32_t source, BFSResult& result) {
    std::vector<uint32_t> current_frontier;
    std::vector<uint32_t> next_frontier;
    
    result.distances[source] = 0;
    result.parents[source] = source;
    current_frontier.push_back(source);
    result.visitation_order.push_back(source);
    
    uint32_t depth = 0;
    
    while (!current_frontier.empty()) {
        next_frontier.clear();
        
        for (uint32_t vertex : current_frontier) {
            uint32_t degree = graph_.getDegree(vertex);
            const uint32_t* neighbors = graph_.getNeighbors(vertex);
            
            for (uint32_t i = 0; i < degree; ++i) {
                uint32_t neighbor = neighbors[i];
                result.edges_visited++;
                
                if (result.distances[neighbor] == -1) {
                    result.distances[neighbor] = depth + 1;
                    result.parents[neighbor] = vertex;
                    result.visitation_order.push_back(neighbor);
                    next_frontier.push_back(neighbor);
                }
            }
        }
        
        current_frontier.swap(next_frontier);
        depth++;
    }
}

void BFS::bfsQueueBased(uint32_t source, BFSResult& result) {
    std::queue<uint32_t> queue;
    
    result.distances[source] = 0;
    result.parents[source] = source;
    queue.push(source);
    result.visitation_order.push_back(source);
    
    while (!queue.empty()) {
        uint32_t vertex = queue.front();
        queue.pop();
        
        uint32_t degree = graph_.getDegree(vertex);
        const uint32_t* neighbors = graph_.getNeighbors(vertex);
        
        for (uint32_t i = 0; i < degree; ++i) {
            uint32_t neighbor = neighbors[i];
            result.edges_visited++;
            
            if (result.distances[neighbor] == -1) {
                result.distances[neighbor] = result.distances[vertex] + 1;
                result.parents[neighbor] = vertex;
                result.visitation_order.push_back(neighbor);
                queue.push(neighbor);
            }
        }
    }
}

bool BFS::validate(const CSRGraph& graph, const BFSResult& result, uint32_t source) {
    if (source >= graph.numVertices()) {
        return false;
    }
    
    // Check source
    if (result.distances[source] != 0 || result.parents[source] != source) {
        return false;
    }
    
    // Check all vertices
    for (uint32_t i = 0; i < graph.numVertices(); ++i) {
        if (result.distances[i] == -1) {
            // Unreachable vertex
            continue;
        }
        
        // Check parent relationship
        if (result.parents[i] != i) {  // not the source
            uint32_t parent = result.parents[i];
            if (parent >= graph.numVertices()) {
                return false;
            }
            
            // Check if parent's distance is one less
            if (result.distances[parent] != result.distances[i] - 1) {
                return false;
            }
            
            // Check if edge exists between parent and vertex
            bool edge_exists = false;
            uint32_t degree = graph.getDegree(parent);
            const uint32_t* neighbors = graph.getNeighbors(parent);
            
            for (uint32_t j = 0; j < degree; ++j) {
                if (neighbors[j] == i) {
                    edge_exists = true;
                    break;
                }
            }
            
            if (!edge_exists) {
                return false;
            }
        }
    }
    
    return true;
}

} // namespace morpheus