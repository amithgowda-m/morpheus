#include "graph_generator.h"
#include <algorithm>
#include <iostream>

namespace morpheus {

GraphGenerator::GraphGenerator(uint32_t seed) : rng_(seed) {}

CSRGraph GraphGenerator::generateRMAT(uint32_t scale, uint32_t edge_factor, 
                                     float a, float b, float c, float d) {
    uint32_t n = 1 << scale;  // 2^scale vertices
    uint32_t m = n * edge_factor;
    
    // Normalize probabilities
    float total = a + b + c + d;
    a /= total;
    b /= total;
    c /= total;
    d /= total;
    
    std::vector<std::pair<uint32_t, uint32_t>> edges;
    edges.reserve(m);
    
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    for (uint32_t i = 0; i < m; ++i) {
        uint32_t x = 0, y = 0;
        uint32_t sx = n, sy = n;
        
        for (uint32_t level = 0; level < scale; ++level) {
            sx >>= 1;
            sy >>= 1;
            
            float r = dist(rng_);
            if (r < a) {
                // quadrant A: do nothing (x, y stay same)
            } else if (r < a + b) {
                x += sx;  // quadrant B
            } else if (r < a + b + c) {
                y += sy;  // quadrant C
            } else {
                x += sx;  // quadrant D
                y += sy;
            }
        }
        
        edges.emplace_back(x, y);
    }
    
    CSRGraph graph;
    graph.buildFromEdges(n, edges);
    return graph;
}

CSRGraph GraphGenerator::generateRandom(uint32_t num_vertices, uint32_t num_edges) {
    std::vector<std::pair<uint32_t, uint32_t>> edges;
    edges.reserve(num_edges);
    
    std::uniform_int_distribution<uint32_t> vertex_dist(0, num_vertices - 1);
    
    for (uint32_t i = 0; i < num_edges; ++i) {
        uint32_t src = vertex_dist(rng_);
        uint32_t dest = vertex_dist(rng_);
        edges.emplace_back(src, dest);
    }
    
    CSRGraph graph;
    graph.buildFromEdges(num_vertices, edges);
    return graph;
}

CSRGraph GraphGenerator::generateGrid(uint32_t width, uint32_t height) {
    uint32_t num_vertices = width * height;
    std::vector<std::pair<uint32_t, uint32_t>> edges;
    
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            uint32_t vertex = y * width + x;
            
            // Right neighbor
            if (x < width - 1) {
                edges.emplace_back(vertex, vertex + 1);
                edges.emplace_back(vertex + 1, vertex);  // undirected
            }
            
            // Down neighbor
            if (y < height - 1) {
                edges.emplace_back(vertex, vertex + width);
                edges.emplace_back(vertex + width, vertex);  // undirected
            }
        }
    }
    
    CSRGraph graph;
    graph.buildFromEdges(num_vertices, edges);
    return graph;
}

CSRGraph GraphGenerator::generateStar(uint32_t num_vertices) {
    if (num_vertices < 2) {
        return CSRGraph();
    }
    
    std::vector<std::pair<uint32_t, uint32_t>> edges;
    
    // Center vertex is 0
    for (uint32_t i = 1; i < num_vertices; ++i) {
        edges.emplace_back(0, i);
        edges.emplace_back(i, 0);  // undirected
    }
    
    CSRGraph graph;
    graph.buildFromEdges(num_vertices, edges);
    return graph;
}

CSRGraph GraphGenerator::generateComplete(uint32_t num_vertices) {
    std::vector<std::pair<uint32_t, uint32_t>> edges;
    
    for (uint32_t i = 0; i < num_vertices; ++i) {
        for (uint32_t j = 0; j < num_vertices; ++j) {
            if (i != j) {
                edges.emplace_back(i, j);
            }
        }
    }
    
    CSRGraph graph;
    graph.buildFromEdges(num_vertices, edges);
    return graph;
}

} // namespace morpheus