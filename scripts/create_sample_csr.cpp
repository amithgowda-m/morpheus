#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

int main() {
    std::ifstream input("sample_graph.txt");
    std::vector<std::pair<int, int>> edges;
    int max_vertex = 0;
    
    std::string line;
    while (std::getline(input, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        int src, dest;
        if (iss >> src >> dest) {
            edges.push_back({src, dest});
            max_vertex = std::max(max_vertex, std::max(src, dest));
        }
    }
    
    // Create CSR-like structure
    std::vector<int> row_ptr(max_vertex + 2, 0);
    std::vector<int> col_idx;
    
    // Count degrees
    for (const auto& edge : edges) {
        row_ptr[edge.first + 1]++;
    }
    
    // Cumulative sum for row_ptr
    for (int i = 1; i <= max_vertex + 1; i++) {
        row_ptr[i] += row_ptr[i - 1];
    }
    
    // Fill col_idx
    col_idx.resize(edges.size());
    std::vector<int> current_pos(max_vertex + 1, 0);
    
    for (const auto& edge : edges) {
        int pos = row_ptr[edge.first] + current_pos[edge.first];
        col_idx[pos] = edge.dest;
        current_pos[edge.first]++;
    }
    
    std::cout << "Graph with " << (max_vertex + 1) << " vertices and " 
              << edges.size() << " edges created." << std::endl;
    return 0;
}
