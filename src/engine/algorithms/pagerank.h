#ifndef MORPHEUS_PAGERANK_H
#define MORPHEUS_PAGERANK_H

#include "../graph/csr_graph.h"
#include <vector>
#include <cmath>

namespace morpheus {

struct PageRankResult {
    std::vector<double> scores;
    uint32_t iterations;
    uint64_t execution_time_ns;
    double final_residual;
    
    PageRankResult(uint32_t num_vertices)
        : scores(num_vertices, 0.0),
          iterations(0),
          execution_time_ns(0),
          final_residual(0.0) {}
};

class PageRank {
public:
    PageRank(const CSRGraph& graph, double damping = 0.85, double tolerance = 1e-8);
    
    // Standard PageRank
    PageRankResult run(uint32_t max_iterations = 100);
    
    // Personalized PageRank
    PageRankResult runPersonalized(const std::vector<double>& personalization_vector,
                                  uint32_t max_iterations = 100);
    
    // Validation
    static bool validate(const CSRGraph& graph, const PageRankResult& result, 
                        double damping = 0.85, double tolerance = 1e-6);

private:
    const CSRGraph& graph_;
    double damping_;
    double tolerance_;
    
    void initialize(std::vector<double>& ranks) const;
    double computeResidual(const std::vector<double>& old_ranks, 
                          const std::vector<double>& new_ranks) const;
};

} // namespace morpheus

#endif // MORPHEUS_PAGERANK_H