#ifndef MORPHEUS_GRAPH_GENERATOR_H
#define MORPHEUS_GRAPH_GENERATOR_H

#include "csr_graph.h"
#include <random>
#include <vector>
#include <cmath>

namespace morpheus {

class GraphGenerator {
public:
    GraphGenerator(uint32_t seed = 42);
    
    // RMAT graph generation
    CSRGraph generateRMAT(uint32_t scale, uint32_t edge_factor, 
                         float a = 0.57f, float b = 0.19f, 
                         float c = 0.19f, float d = 0.05f);
    
    // Random graph
    CSRGraph generateRandom(uint32_t num_vertices, uint32_t num_edges);
    
    // Grid graph
    CSRGraph generateGrid(uint32_t width, uint32_t height);
    
    // Star graph
    CSRGraph generateStar(uint32_t num_vertices);
    
    // Complete graph
    CSRGraph generateComplete(uint32_t num_vertices);

private:
    std::mt19937 rng_;
    
    void rmatRecursive(uint32_t n, uint32_t x, uint32_t y, uint32_t sx, uint32_t sy,
                      float a, float b, float c, float d,
                      std::vector<std::pair<uint32_t, uint32_t>>& edges);
};

} // namespace morpheus

#endif // MORPHEUS_GRAPH_GENERATOR_H