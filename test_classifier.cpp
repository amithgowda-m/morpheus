#include "src/engine/monitoring/trained_classifier.h"
#include <iostream>
#include <vector>

using namespace morpheus;

void test_classifier() {
    std::cout << "Testing Trained Phase Classifier..." << std::endl;
    
    // Test case 1: Dense Sequential pattern
    std::vector<double> dense_features = {0.002, 1.9, 0.015, 800, 300, 950000, 500000};
    ExecutionPhase phase1 = TrainedPhaseClassifier::classify(dense_features);
    std::cout << "Dense features -> Phase: " << static_cast<int>(phase1) << " (expected: 0)" << std::endl;
    
    // Test case 2: Sparse Random pattern  
    std::vector<double> sparse_features = {0.025, 0.8, 0.028, 4500, 2500, 1100000, 1400000};
    ExecutionPhase phase2 = TrainedPhaseClassifier::classify(sparse_features);
    std::cout << "Sparse features -> Phase: " << static_cast<int>(phase2) << " (expected: 1)" << std::endl;
    
    // Test case 3: Pointer Chasing pattern
    std::vector<double> pointer_features = {0.012, 0.95, 0.075, 1800, 900, 1000000, 1050000};
    ExecutionPhase phase3 = TrainedPhaseClassifier::classify(pointer_features);
    std::cout << "Pointer features -> Phase: " << static_cast<int>(phase3) << " (expected: 2)" << std::endl;
    
    std::cout << "Classifier test completed!" << std::endl;
}

int main() {
    test_classifier();
    return 0;
}
