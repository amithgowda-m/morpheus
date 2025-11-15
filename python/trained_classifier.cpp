// Auto-generated decision tree classifier
// Features: l3_miss_rate, ipc, branch_miss_rate, l1_misses, l2_misses, instructions, cycles
// Classes: DenseSequential, PointerChasing, SparseRandom

#include <vector>
#include <string>
#include <cmath>

namespace morpheus {

enum class ExecutionPhase {
    DenseSequential, SparseRandom, PointerChasing, Unknown
};

class TrainedPhaseClassifier {
public:
    static ExecutionPhase classify(const std::vector<double>& features) {
        if (features.size() < 7) {
            return ExecutionPhase::Unknown;
        }
        
        // Extract features
        double l3_miss_rate = features[0], ipc = features[1], branch_miss_rate = features[2], l1_misses = features[3], l2_misses = features[4], instructions = features[5], cycles = features[6];
        
        // Decision tree rules
        if (features[1] <= 1.267921) {
            if (features[2] <= 0.048740) {
                if (features[0] <= 0.008015) {
                    if (features[1] <= 0.909520) {
                        return ExecutionPhase::SparseRandom;
                    } else {
                        if (features[2] <= 0.034531) {
                            return ExecutionPhase::DenseSequential;
                        } else {
                            return ExecutionPhase::DenseSequential;
                        }
                    }
                } else {
                    if (features[0] <= 0.019926) {
                        if (features[2] <= 0.040511) {
                            return ExecutionPhase::SparseRandom;
                        } else {
                            return ExecutionPhase::PointerChasing;
                        }
                    } else {
                        if (features[0] <= 0.023621) {
                            return ExecutionPhase::SparseRandom;
                        } else {
                            return ExecutionPhase::SparseRandom;
                        }
                    }
                }
            } else {
                if (features[0] <= 0.032926) {
                    if (features[0] <= 0.023788) {
                        if (features[2] <= 0.053023) {
                            return ExecutionPhase::PointerChasing;
                        } else {
                            return ExecutionPhase::PointerChasing;
                        }
                    } else {
                        if (features[1] <= 0.747415) {
                            return ExecutionPhase::SparseRandom;
                        } else {
                            return ExecutionPhase::PointerChasing;
                        }
                    }
                } else {
                    return ExecutionPhase::SparseRandom;
                }
            }
        } else {
            if (features[2] <= 0.051773) {
                if (features[2] <= 0.046426) {
                    return ExecutionPhase::DenseSequential;
                } else {
                    return ExecutionPhase::DenseSequential;
                }
            } else {
                if (features[6] <= 725101.875000) {
                    return ExecutionPhase::PointerChasing;
                } else {
                    return ExecutionPhase::PointerChasing;
                }
            }
        }

    }
    
private:
    static constexpr double L3_MISS_THRESHOLD = 0.01;
    static constexpr double IPC_THRESHOLD = 1.0;
    static constexpr double BRANCH_MISS_THRESHOLD = 0.05;
};

} // namespace morpheus
