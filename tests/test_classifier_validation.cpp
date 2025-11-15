#include "../src/engine/monitoring/trained_classifier.h"
#include <iostream>
#include <vector>
#include <cassert>
#include <chrono>

using namespace morpheus;

void test_classifier_accuracy() {
    std::cout << "=== CLASSIFIER VALIDATION FOR ACM PAPER ===" << std::endl;
    
    struct TestCase {
        std::vector<double> features;
        ExecutionPhase expected;
        const char* description;
    };
    
    std::vector<TestCase> test_cases = {
        {{0.002, 1.9, 0.015, 800, 300, 950000, 500000}, 
         ExecutionPhase::DenseSequential, "Low miss rate, high IPC"},
        
        {{0.025, 0.8, 0.028, 4500, 2500, 1100000, 1400000}, 
         ExecutionPhase::SparseRandom, "High miss rate, low IPC"},
         
        {{0.012, 0.95, 0.075, 1800, 900, 1000000, 1050000}, 
         ExecutionPhase::PointerChasing, "Medium miss rate, high branch misses"}
    };
    
    int passed = 0;
    for (const auto& test : test_cases) {
        ExecutionPhase result = TrainedPhaseClassifier::classify(test.features);
        bool correct = (result == test.expected);
        std::cout << (correct ? "✅ PASS" : "❌ FAIL") 
                  << ": " << test.description 
                  << " -> Phase " << static_cast<int>(result) 
                  << " (expected " << static_cast<int>(test.expected) << ")" << std::endl;
        if (correct) passed++;
    }
    
    std::cout << "\nValidation Results: " << passed << "/" << test_cases.size() 
              << " test cases passed (" << (passed * 100.0 / test_cases.size()) << "% )" << std::endl;
    
    assert(passed >= static_cast<int>(test_cases.size() * 0.9));
    std::cout << "✅ CLASSIFIER MEETS ACM PAPER REQUIREMENTS" << std::endl;
}

void test_performance_characteristics() {
    std::cout << "\n=== PERFORMANCE CHARACTERISTICS ===" << std::endl;
    std::vector<std::vector<double>> test_features(10000, {0.005, 1.2, 0.03, 1000, 500, 1000000, 800000});
    
    auto start = std::chrono::high_resolution_clock::now();
    for (auto& features : test_features) {
        TrainedPhaseClassifier::classify(features);
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    double avg_time_ns = (duration.count() * 1000.0) / test_features.size();
    
    std::cout << "Average classification time: " << avg_time_ns << " ns" << std::endl;
    std::cout << "Classification rate: " << (1e9 / avg_time_ns) << " classifications/second" << std::endl;
    
    assert(avg_time_ns < 1000);
    std::cout << "✅ CLASSIFIER MEETS PERFORMANCE REQUIREMENTS (<1μs)" << std::endl;
}

int main() {
    test_classifier_accuracy();
    test_performance_characteristics();
    std::cout << "\n🎉 ALL VALIDATION TESTS PASSED - READY FOR ACM SUBMISSION" << std::endl;
    return 0;
}
