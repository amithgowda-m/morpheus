#ifndef MORPHEUS_BFS_H
#define MORPHEUS_BFS_H

#include "../graph/csr_graph.h"
#include <vector>
#include <queue>
#include <cstdint>

namespace morpheus {

struct BFSResult {
    std::vector<int32_t> distances;
    std::vector<uint32_t> parents;
    std::vector<uint32_t> visitation_order;
    uint64_t execution_time_ns;
    uint64_t edges_visited;
    
    BFSResult(uint32_t num_vertices) 
        : distances(num_vertices, -1), 
          parents(num_vertices, UINT32_MAX),
          execution_time_ns(0),
          edges_visited(0) {}
};

class BFS {
public:
    BFS(const CSRGraph& graph);
    
    // Standard BFS
    BFSResult run(uint32_t source_vertex);
    
    // BFS with early termination
    BFSResult runUntil(uint32_t source_vertex, uint32_t max_depth);
    
    // Multi-source BFS
    BFSResult runMultiSource(const std::vector<uint32_t>& sources);
    
    // Validation
    static bool validate(const CSRGraph& graph, const BFSResult& result, uint32_t source);

private:
    const CSRGraph& graph_;
    
    void bfsLevelSync(uint32_t source, BFSResult& result);
    void bfsQueueBased(uint32_t source, BFSResult& result);
};

} // namespace morpheus

#endif // MORPHEUS_BFS_H