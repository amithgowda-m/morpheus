#ifndef MORPHEUS_BETWEENNESS_H
#define MORPHEUS_BETWEENNESS_H

#include "../graph/csr_graph.h"
#include <vector>
#include <queue>
#include <stack>

namespace morpheus {

struct BetweennessResult {
    std::vector<double> centrality;
    uint64_t execution_time_ns;
    uint64_t shortest_paths_count;
    
    BetweennessResult(uint32_t num_vertices)
        : centrality(num_vertices, 0.0),
          execution_time_ns(0),
          shortest_paths_count(0) {}
};

class BetweennessCentrality {
public:
    BetweennessCentrality(const CSRGraph& graph);
    
    // Standard betweenness centrality (Brandes algorithm)
    BetweennessResult run(bool normalized = true);
    
    // Approximate betweenness with sampling
    BetweennessResult runApproximate(uint32_t sample_size, bool normalized = true);
    
    // Vertex betweenness for single source
    std::vector<double> runSingleSource(uint32_t source);
    
    // Validation
    static bool validate(const CSRGraph& graph, const BetweennessResult& result, 
                        double tolerance = 1e-6);

private:
    const CSRGraph& graph_;
    
    void brandesAlgorithm(std::vector<double>& centrality, bool normalized);
};

} // namespace morpheus

#endif // MORPHEUS_BETWEENNESS_H