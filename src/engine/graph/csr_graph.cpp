#include "csr_graph.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <unordered_map>

namespace morpheus {

CSRGraph::CSRGraph() 
    : num_vertices_(0), num_edges_(0), weighted_(false) {}

CSRGraph::~CSRGraph() {}

bool CSRGraph::loadFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open graph file: " << filename << std::endl;
        return false;
    }
    
    // Read header
    char magic[4];
    file.read(magic, 4);
    if (std::string(magic, 4) != "CSR\x01") {
        std::cerr << "Invalid CSR file format" << std::endl;
        return false;
    }
    
    file.read(reinterpret_cast<char*>(&num_vertices_), sizeof(uint32_t));
    file.read(reinterpret_cast<char*>(&num_edges_), sizeof(uint32_t));
    file.read(reinterpret_cast<char*>(&weighted_), sizeof(bool));
    
    // Read data arrays
    row_ptr_.resize(num_vertices_ + 1);
    col_idx_.resize(num_edges_);
    
    file.read(reinterpret_cast<char*>(row_ptr_.data()), (num_vertices_ + 1) * sizeof(uint32_t));
    file.read(reinterpret_cast<char*>(col_idx_.data()), num_edges_ * sizeof(uint32_t));
    
    if (weighted_) {
        weights_.resize(num_edges_);
        file.read(reinterpret_cast<char*>(weights_.data()), num_edges_ * sizeof(float));
    }
    
    return file.good();
}

bool CSRGraph::loadFromMTX(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open MTX file: " << filename << std::endl;
        return false;
    }
    
    std::string line;
    
    // Skip comments
    while (std::getline(file, line)) {
        if (line[0] != '%') break;
    }
    
    // Read dimensions
    uint32_t num_rows, num_cols, num_entries;
    std::istringstream iss(line);
    iss >> num_rows >> num_cols >> num_entries;
    
    if (num_rows != num_cols) {
        std::cerr << "Only square matrices supported" << std::endl;
        return false;
    }
    
    std::vector<std::tuple<uint32_t, uint32_t, float>> edges;
    edges.reserve(num_entries);
    
    weighted_ = false;
    uint32_t max_vertex = 0;
    
    // Read edges (MTX uses 1-based indexing)
    for (uint32_t i = 0; i < num_entries; ++i) {
        std::getline(file, line);
        std::istringstream edge_iss(line);
        
        uint32_t src, dest;
        float weight = 1.0f;
        
        edge_iss >> src >> dest;
        if (edge_iss >> weight) {
            weighted_ = true;
        }
        
        // Convert to 0-based
        src--;
        dest--;
        
        edges.emplace_back(src, dest, weight);
        max_vertex = std::max(max_vertex, std::max(src, dest));
    }
    
    num_vertices_ = max_vertex + 1;
    buildFromWeightedEdges(num_vertices_, edges);
    
    return true;
}

void CSRGraph::buildFromEdges(uint32_t num_vertices, 
                             const std::vector<std::pair<uint32_t, uint32_t>>& edges) {
    std::vector<uint32_t> src_vertices, dest_vertices;
    src_vertices.reserve(edges.size());
    dest_vertices.reserve(edges.size());
    
    for (const auto& edge : edges) {
        src_vertices.push_back(edge.first);
        dest_vertices.push_back(edge.second);
    }
    
    buildCSR(num_vertices, src_vertices, dest_vertices);
}

void CSRGraph::buildFromWeightedEdges(uint32_t num_vertices,
                                     const std::vector<std::tuple<uint32_t, uint32_t, float>>& edges) {
    std::vector<uint32_t> src_vertices, dest_vertices;
    std::vector<float> edge_weights;
    
    src_vertices.reserve(edges.size());
    dest_vertices.reserve(edges.size());
    edge_weights.reserve(edges.size());
    
    for (const auto& edge : edges) {
        src_vertices.push_back(std::get<0>(edge));
        dest_vertices.push_back(std::get<1>(edge));
        edge_weights.push_back(std::get<2>(edge));
    }
    
    buildCSR(num_vertices, src_vertices, dest_vertices, edge_weights);
}

void CSRGraph::buildCSR(uint32_t num_vertices, 
                       const std::vector<uint32_t>& src_vertices,
                       const std::vector<uint32_t>& dest_vertices,
                       const std::vector<float>& edge_weights) {
    num_vertices_ = num_vertices;
    num_edges_ = src_vertices.size();
    weighted_ = !edge_weights.empty();
    
    if (weighted_ && edge_weights.size() != num_edges_) {
        throw std::invalid_argument("Edge weights size mismatch");
    }
    
    // Count degrees
    std::vector<uint32_t> degrees(num_vertices_, 0);
    for (uint32_t src : src_vertices) {
        if (src < num_vertices_) {
            degrees[src]++;
        }
    }
    
    // Build row pointer
    row_ptr_.resize(num_vertices_ + 1);
    row_ptr_[0] = 0;
    for (uint32_t i = 0; i < num_vertices_; ++i) {
        row_ptr_[i + 1] = row_ptr_[i] + degrees[i];
    }
    
    // Build column index and weights
    col_idx_.resize(num_edges_);
    if (weighted_) {
        weights_.resize(num_edges_);
    }
    
    std::vector<uint32_t> current_pos(num_vertices_, 0);
    
    for (size_t i = 0; i < num_edges_; ++i) {
        uint32_t src = src_vertices[i];
        uint32_t dest = dest_vertices[i];
        
        if (src < num_vertices_) {
            uint32_t pos = row_ptr_[src] + current_pos[src];
            col_idx_[pos] = dest;
            if (weighted_) {
                weights_[pos] = edge_weights[i];
            }
            current_pos[src]++;
        }
    }
}

uint32_t CSRGraph::getDegree(uint32_t vertex) const {
    if (vertex >= num_vertices_) {
        return 0;
    }
    return row_ptr_[vertex + 1] - row_ptr_[vertex];
}

const uint32_t* CSRGraph::getNeighbors(uint32_t vertex) const {
    if (vertex >= num_vertices_) {
        return nullptr;
    }
    return col_idx_.data() + row_ptr_[vertex];
}

const float* CSRGraph::getWeights(uint32_t vertex) const {
    if (vertex >= num_vertices_ || !weighted_) {
        return nullptr;
    }
    return weights_.data() + row_ptr_[vertex];
}

bool CSRGraph::validate() const {
    if (row_ptr_.size() != num_vertices_ + 1) {
        return false;
    }
    
    if (row_ptr_[0] != 0 || row_ptr_[num_vertices_] != num_edges_) {
        return false;
    }
    
    for (uint32_t i = 0; i < num_vertices_; ++i) {
        if (row_ptr_[i] > row_ptr_[i + 1]) {
            return false;
        }
    }
    
    for (uint32_t edge : col_idx_) {
        if (edge >= num_vertices_) {
            return false;
        }
    }
    
    return true;
}

size_t CSRGraph::getMemoryUsage() const {
    size_t usage = 0;
    usage += row_ptr_.size() * sizeof(uint32_t);
    usage += col_idx_.size() * sizeof(uint32_t);
    usage += weights_.size() * sizeof(float);
    return usage;
}

} // namespace morpheus