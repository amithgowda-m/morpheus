#ifndef MORPHEUS_PHASE_CLASSIFIER_H
#define MORPHEUS_PHASE_CLASSIFIER_H

#include "performance_monitor.h"
#include <vector>
#include <string>

namespace morpheus {

// This header contains the decision tree rules exported from Python training
class PhaseClassifier {
public:
    static ExecutionPhase classify(const std::vector<double>& features);
    
    // Feature names for interpretation
    static const std::vector<std::string> feature_names;
    
    // Phase names for reporting
    static const std::vector<std::string> phase_names;

private:
    // Decision tree rules (simplified version - would be auto-generated)
    static constexpr double L3_MISS_RATE_THRESHOLD = 0.015;
    static constexpr double IPC_THRESHOLD_LOW = 0.8;
    static constexpr double IPC_THRESHOLD_HIGH = 1.2;
    static constexpr double BRANCH_MISS_RATE_THRESHOLD = 0.03;
    static constexpr double L1_MISS_RATE_THRESHOLD = 0.1;
};

} // namespace morpheus

#endif // MORPHEUS_PHASE_CLASSIFIER_H