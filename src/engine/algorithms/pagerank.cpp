#include "pagerank.h"
#include <chrono>
#include <iostream>
#include <numeric>

namespace morpheus {

PageRank::PageRank(const CSRGraph& graph, double damping, double tolerance)
    : graph_(graph), damping_(damping), tolerance_(tolerance) {
    
    if (damping_ <= 0.0 || damping_ >= 1.0) {
        throw std::invalid_argument("Damping factor must be between 0 and 1");
    }
}

PageRankResult PageRank::run(uint32_t max_iterations) {
    PageRankResult result(graph_.numVertices());
    auto start = std::chrono::steady_clock::now();
    
    std::vector<double> old_ranks(graph_.numVertices());
    std::vector<double> new_ranks(graph_.numVertices());
    
    initialize(old_ranks);
    
    const double base_rank = (1.0 - damping_) / graph_.numVertices();
    uint32_t iter;
    
    for (iter = 0; iter < max_iterations; ++iter) {
        // Initialize new ranks with base rank
        std::fill(new_ranks.begin(), new_ranks.end(), base_rank);
        
        // Distribute ranks
        for (uint32_t i = 0; i < graph_.numVertices(); ++i) {
            uint32_t degree = graph_.getDegree(i);
            if (degree > 0) {
                double contribution = damping_ * old_ranks[i] / degree;
                const uint32_t* neighbors = graph_.getNeighbors(i);
                
                for (uint32_t j = 0; j < degree; ++j) {
                    uint32_t neighbor = neighbors[j];
                    new_ranks[neighbor] += contribution;
                }
            } else {
                // Handle dangling nodes by redistributing their rank
                double dangling_contribution = damping_ * old_ranks[i] / graph_.numVertices();
                for (uint32_t j = 0; j < graph_.numVertices(); ++j) {
                    new_ranks[j] += dangling_contribution;
                }
            }
        }
        
        // Check convergence
        double residual = computeResidual(old_ranks, new_ranks);
        if (residual < tolerance_) {
            break;
        }
        
        old_ranks.swap(new_ranks);
    }
    
    result.scores = std::move(old_ranks);
    result.iterations = iter + 1;
    {
        auto end = std::chrono::steady_clock::now();
        result.execution_time_ns = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
    }
    result.final_residual = computeResidual(old_ranks, new_ranks);
    
    return result;
}

PageRankResult PageRank::runPersonalized(const std::vector<double>& personalization_vector,
                                        uint32_t max_iterations) {
    if (personalization_vector.size() != graph_.numVertices()) {
        throw std::invalid_argument("Personalization vector size mismatch");
    }
    
    double sum = std::accumulate(personalization_vector.begin(), 
                                personalization_vector.end(), 0.0);
    if (std::abs(sum - 1.0) > 1e-6) {
        throw std::invalid_argument("Personalization vector must sum to 1");
    }
    
    PageRankResult result(graph_.numVertices());
    auto start = std::chrono::steady_clock::now();
    
    std::vector<double> old_ranks(graph_.numVertices());
    std::vector<double> new_ranks(graph_.numVertices());
    
    // Initialize with personalization vector
    old_ranks = personalization_vector;
    
    uint32_t iter;
    
    for (iter = 0; iter < max_iterations; ++iter) {
        std::fill(new_ranks.begin(), new_ranks.end(), 0.0);
        
        // Distribute ranks with personalization
        for (uint32_t i = 0; i < graph_.numVertices(); ++i) {
            uint32_t degree = graph_.getDegree(i);
            if (degree > 0) {
                double contribution = damping_ * old_ranks[i] / degree;
                const uint32_t* neighbors = graph_.getNeighbors(i);
                
                for (uint32_t j = 0; j < degree; ++j) {
                    uint32_t neighbor = neighbors[j];
                    new_ranks[neighbor] += contribution;
                }
            } else {
                // Dangling node handling
                double dangling_contribution = damping_ * old_ranks[i] / graph_.numVertices();
                for (uint32_t j = 0; j < graph_.numVertices(); ++j) {
                    new_ranks[j] += dangling_contribution;
                }
            }
        }
        
        // Add personalization vector
        for (uint32_t i = 0; i < graph_.numVertices(); ++i) {
            new_ranks[i] += (1.0 - damping_) * personalization_vector[i];
        }
        
        // Check convergence
        double residual = computeResidual(old_ranks, new_ranks);
        if (residual < tolerance_) {
            break;
        }
        
        old_ranks.swap(new_ranks);
    }
    
    result.scores = std::move(old_ranks);
    result.iterations = iter + 1;
    {
        auto end = std::chrono::steady_clock::now();
        result.execution_time_ns = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
    }
    result.final_residual = computeResidual(old_ranks, new_ranks);
    
    return result;
}

void PageRank::initialize(std::vector<double>& ranks) const {
    double initial_rank = 1.0 / graph_.numVertices();
    std::fill(ranks.begin(), ranks.end(), initial_rank);
}

double PageRank::computeResidual(const std::vector<double>& old_ranks, 
                                const std::vector<double>& new_ranks) const {
    double residual = 0.0;
    for (size_t i = 0; i < old_ranks.size(); ++i) {
        double diff = std::abs(new_ranks[i] - old_ranks[i]);
        residual = std::max(residual, diff);
    }
    return residual;
}

bool PageRank::validate(const CSRGraph& graph, const PageRankResult& result, 
                       double damping, double tolerance) {
    // Check that scores sum to approximately 1
    double sum = 0.0;
    for (double score : result.scores) {
        sum += score;
    }
    
    if (std::abs(sum - 1.0) > tolerance) {
        return false;
    }
    
    // Check that all scores are non-negative
    for (double score : result.scores) {
        if (score < -tolerance) {
            return false;
        }
    }
    
    return true;
}

} // namespace morpheus