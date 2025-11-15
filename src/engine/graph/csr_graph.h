#ifndef MORPHEUS_CSR_GRAPH_H
#define MORPHEUS_CSR_GRAPH_H

#include <vector>
#include <cstdint>
#include <string>
#include <fstream>
#include <stdexcept>

namespace morpheus {

struct CSREdge {
    uint32_t dest;
    float weight;
    
    CSREdge(uint32_t d, float w = 1.0f) : dest(d), weight(w) {}
};

class CSRGraph {
public:
    CSRGraph();
    ~CSRGraph();
    
    // Load graph from file
    bool loadFromFile(const std::string& filename);
    bool loadFromMTX(const std::string& filename);
    
    // Build from edge list
    void buildFromEdges(uint32_t num_vertices, 
                       const std::vector<std::pair<uint32_t, uint32_t>>& edges);
    void buildFromWeightedEdges(uint32_t num_vertices,
                               const std::vector<std::tuple<uint32_t, uint32_t, float>>& edges);
    
    // Graph properties
    uint32_t numVertices() const { return num_vertices_; }
    uint32_t numEdges() const { return num_edges_; }
    bool isWeighted() const { return weighted_; }
    
    // Accessors
    const std::vector<uint32_t>& rowPtr() const { return row_ptr_; }
    const std::vector<uint32_t>& colIdx() const { return col_idx_; }
    const std::vector<float>& weights() const { return weights_; }
    
    // Vertex operations
    uint32_t getDegree(uint32_t vertex) const;
    const uint32_t* getNeighbors(uint32_t vertex) const;
    const float* getWeights(uint32_t vertex) const;
    
    // Validation
    bool validate() const;
    
    // Memory usage
    size_t getMemoryUsage() const;

private:
    uint32_t num_vertices_;
    uint32_t num_edges_;
    bool weighted_;
    
    std::vector<uint32_t> row_ptr_;
    std::vector<uint32_t> col_idx_;
    std::vector<float> weights_;
    
    void buildCSR(uint32_t num_vertices, 
                 const std::vector<uint32_t>& src_vertices,
                 const std::vector<uint32_t>& dest_vertices,
                 const std::vector<float>& edge_weights = {});
};

} // namespace morpheus

#endif // MORPHEUS_CSR_GRAPH_H