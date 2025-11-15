#include "../src/engine/graph/csr_graph.h"
#include "../src/engine/algorithms/bfs.h"
#include "../src/engine/algorithms/pagerank.h"
#include "../src/engine/algorithms/betweenness.h"
#include "../src/engine/monitoring/performance_monitor.h"
#include "../src/utils/config_parser.h"
#include "../src/utils/result_writer.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <chrono>
#include <cstring>

using namespace morpheus;

struct BenchmarkConfig {
    std::string graph_file;
    std::string algorithm;
    int iterations;
    bool validate;
    bool with_monitoring;
    std::string output_file;
};

BenchmarkConfig parseCommandLine(int argc, char* argv[]) {
    BenchmarkConfig config;
    config.iterations = 5;
    config.validate = false;
    config.with_monitoring = false;
    
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--graph") == 0 && i + 1 < argc) {
            config.graph_file = argv[++i];
        } else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            config.algorithm = argv[++i];
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            config.iterations = std::atoi(argv[++i]);
        } else if (strcmp(argv[i], "--validate") == 0) {
            config.validate = true;
        } else if (strcmp(argv[i], "--with-monitoring") == 0) {
            config.with_monitoring = true;
        } else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            config.output_file = argv[++i];
        } else if (strcmp(argv[i], "--help") == 0) {
            std::cout << "Usage: " << argv[0] << " [OPTIONS]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --graph FILE        Graph file to load" << std::endl;
            std::cout << "  --algorithm ALG     Algorithm to run (bfs, pagerank, betweenness)" << std::endl;
            std::cout << "  --iterations N      Number of iterations (default: 5)" << std::endl;
            std::cout << "  --validate          Validate results" << std::endl;
            std::cout << "  --with-monitoring   Enable performance monitoring" << std::endl;
            std::cout << "  --output FILE       Output file for results" << std::endl;
            std::cout << "  --help              Show this help message" << std::endl;
            exit(0);
        }
    }
    
    return config;
}

std::map<std::string, std::string> runBFSBenchmark(const CSRGraph& graph, const BenchmarkConfig& config) {
    std::cout << "Running BFS benchmark..." << std::endl;
    
    BFS bfs(graph);
    std::vector<uint64_t> execution_times;
    uint32_t source_vertex = 0;  // Use first vertex as source
    
    PerformanceMonitor monitor;
    if (config.with_monitoring) {
        monitor.initialize(1);
        monitor.startMonitoring();
    }
    
    for (int i = 0; i < config.iterations; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        
        BFSResult result = bfs.run(source_vertex);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        execution_times.push_back(duration.count());
        
        if (config.validate && !BFS::validate(graph, result, source_vertex)) {
            std::cerr << "BFS validation failed!" << std::endl;
        }
    }
    
    if (config.with_monitoring) {
        monitor.stopMonitoring();
    }
    
    // Calculate statistics
    uint64_t total_time = 0;
    uint64_t min_time = UINT64_MAX;
    uint64_t max_time = 0;
    
    for (uint64_t time : execution_times) {
        total_time += time;
        min_time = std::min(min_time, time);
        max_time = std::max(max_time, time);
    }
    
    uint64_t avg_time = total_time / execution_times.size();
    
    std::map<std::string, std::string> results;
    results["algorithm"] = "bfs";
    results["iterations"] = std::to_string(config.iterations);
    results["min_time_ns"] = std::to_string(min_time);
    results["max_time_ns"] = std::to_string(max_time);
    results["avg_time_ns"] = std::to_string(avg_time);
    results["execution_time_ms"] = std::to_string(avg_time / 1e6);
    results["source_vertex"] = std::to_string(source_vertex);
    
    if (config.with_monitoring) {
        const auto& samples = monitor.getSamples();
        results["performance_samples"] = std::to_string(samples.size());
        
        if (!samples.empty()) {
            results["final_phase"] = std::to_string(static_cast<int>(samples.back().phase));
        }
    }
    
    return results;
}

std::map<std::string, std::string> runPageRankBenchmark(const CSRGraph& graph, const BenchmarkConfig& config) {
    std::cout << "Running PageRank benchmark..." << std::endl;
    
    PageRank pagerank(graph, 0.85, 1e-8);
    std::vector<uint64_t> execution_times;
    std::vector<uint32_t> iteration_counts;
    
    PerformanceMonitor monitor;
    if (config.with_monitoring) {
        monitor.initialize(1);
        monitor.startMonitoring();
    }
    
    for (int i = 0; i < config.iterations; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        
        PageRankResult result = pagerank.run(100);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        execution_times.push_back(duration.count());
        iteration_counts.push_back(result.iterations);
        
        if (config.validate && !PageRank::validate(graph, result, 0.85, 1e-6)) {
            std::cerr << "PageRank validation failed!" << std::endl;
        }
    }
    
    if (config.with_monitoring) {
        monitor.stopMonitoring();
    }
    
    // Calculate statistics
    uint64_t total_time = 0;
    uint64_t min_time = UINT64_MAX;
    uint64_t max_time = 0;
    uint32_t total_iterations = 0;
    
    for (uint64_t time : execution_times) {
        total_time += time;
        min_time = std::min(min_time, time);
        max_time = std::max(max_time, time);
    }
    
    for (uint32_t iter : iteration_counts) {
        total_iterations += iter;
    }
    
    uint64_t avg_time = total_time / execution_times.size();
    uint32_t avg_iterations = total_iterations / iteration_counts.size();
    
    std::map<std::string, std::string> results;
    results["algorithm"] = "pagerank";
    results["iterations"] = std::to_string(config.iterations);
    results["min_time_ns"] = std::to_string(min_time);
    results["max_time_ns"] = std::to_string(max_time);
    results["avg_time_ns"] = std::to_string(avg_time);
    results["execution_time_ms"] = std::to_string(avg_time / 1e6);
    results["avg_convergence_iterations"] = std::to_string(avg_iterations);
    
    if (config.with_monitoring) {
        const auto& samples = monitor.getSamples();
        results["performance_samples"] = std::to_string(samples.size());
    }
    
    return results;
}

std::map<std::string, std::string> runBetweennessBenchmark(const CSRGraph& graph, const BenchmarkConfig& config) {
    std::cout << "Running Betweenness Centrality benchmark..." << std::endl;
    
    BetweennessCentrality bc(graph);
    std::vector<uint64_t> execution_times;
    
    PerformanceMonitor monitor;
    if (config.with_monitoring) {
        monitor.initialize(1);
        monitor.startMonitoring();
    }
    
    for (int i = 0; i < config.iterations; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        
        auto result = bc.runApproximate(10);  // Sample 10 vertices
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        execution_times.push_back(duration.count());
        
        if (config.validate && !BetweennessCentrality::validate(graph, result, 1e-6)) {
            std::cerr << "Betweenness validation failed!" << std::endl;
        }
    }
    
    if (config.with_monitoring) {
        monitor.stopMonitoring();
    }
    
    // Calculate statistics
    uint64_t total_time = 0;
    uint64_t min_time = UINT64_MAX;
    uint64_t max_time = 0;
    
    for (uint64_t time : execution_times) {
        total_time += time;
        min_time = std::min(min_time, time);
        max_time = std::max(max_time, time);
    }
    
    uint64_t avg_time = total_time / execution_times.size();
    
    std::map<std::string, std::string> results;
    results["algorithm"] = "betweenness";
    results["iterations"] = std::to_string(config.iterations);
    results["min_time_ns"] = std::to_string(min_time);
    results["max_time_ns"] = std::to_string(max_time);
    results["avg_time_ns"] = std::to_string(avg_time);
    results["execution_time_ms"] = std::to_string(avg_time / 1e6);
    results["sample_size"] = "10";
    
    if (config.with_monitoring) {
        const auto& samples = monitor.getSamples();
        results["performance_samples"] = std::to_string(samples.size());
    }
    
    return results;
}

int main(int argc, char* argv[]) {
    BenchmarkConfig config = parseCommandLine(argc, argv);
    
    if (config.graph_file.empty()) {
        std::cerr << "Error: Graph file is required" << std::endl;
        std::cerr << "Use --help for usage information" << std::endl;
        return 1;
    }
    
    if (config.algorithm.empty()) {
        std::cerr << "Error: Algorithm is required" << std::endl;
        std::cerr << "Use --help for usage information" << std::endl;
        return 1;
    }
    
    // Load graph
    std::cout << "Loading graph from: " << config.graph_file << std::endl;
    CSRGraph graph;
    
    if (config.graph_file.find(".mtx") != std::string::npos) {
        if (!graph.loadFromMTX(config.graph_file)) {
            std::cerr << "Failed to load MTX graph" << std::endl;
            return 1;
        }
    } else {
        if (!graph.loadFromFile(config.graph_file)) {
            std::cerr << "Failed to load graph file" << std::endl;
            return 1;
        }
    }
    
    std::cout << "Graph loaded: " << graph.numVertices() << " vertices, " 
              << graph.numEdges() << " edges" << std::endl;
    
    if (!graph.validate()) {
        std::cerr << "Graph validation failed!" << std::endl;
        return 1;
    }
    
    // Run benchmark
    std::map<std::string, std::string> results;
    
    if (config.algorithm == "bfs") {
        results = runBFSBenchmark(graph, config);
    } else if (config.algorithm == "pagerank") {
        results = runPageRankBenchmark(graph, config);
    } else if (config.algorithm == "betweenness") {
        results = runBetweennessBenchmark(graph, config);
    } else {
        std::cerr << "Unknown algorithm: " << config.algorithm << std::endl;
        return 1;
    }
    
    // Add common metadata
    results["graph_vertices"] = std::to_string(graph.numVertices());
    results["graph_edges"] = std::to_string(graph.numEdges());
    results["graph_memory_bytes"] = std::to_string(graph.getMemoryUsage());
    results["timestamp"] = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    
    // Output results
    std::cout << "\n=== BENCHMARK RESULTS ===" << std::endl;
    for (const auto& [key, value] : results) {
        std::cout << key << ": " << value << std::endl;
    }
    
    // Write to file if specified
    if (!config.output_file.empty()) {
        ResultWriter writer;
        if (writer.writeToJSON(config.output_file, results)) {
            std::cout << "Results written to: " << config.output_file << std::endl;
        } else {
            std::cerr << "Failed to write results to file" << std::endl;
        }
    }
    
    return 0;
}