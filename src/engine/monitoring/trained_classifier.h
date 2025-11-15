#ifndef MORPHEUS_TRAINED_CLASSIFIER_H
#define MORPHEUS_TRAINED_CLASSIFIER_H

#include <vector>
#include <cmath>

namespace morpheus {

enum class ExecutionPhase {
    DenseSequential,
    SparseRandom,
    PointerChasing,
    Unknown
};

class TrainedPhaseClassifier {
public:
    static ExecutionPhase classify(const std::vector<double>& features) {
        if (features.size() < 7) {
            return ExecutionPhase::Unknown;
        }

        // Extract features
        double l3_miss_rate = features[0];
        double ipc = features[1];
        double branch_miss_rate = features[2];
        double l1_misses = features[3];
        double l2_misses = features[4];
        double instructions = features[5];
        double cycles = features[6];

        // Auto-generated decision tree rules
        if (l3_miss_rate <= 0.008) {
            if (ipc <= 1.218) {
                if (branch_miss_rate <= 0.043) {
                    return ExecutionPhase::SparseRandom;
                } else {
                    if (l3_miss_rate <= 0.004) {
                        return ExecutionPhase::DenseSequential;
                    } else {
                        return ExecutionPhase::PointerChasing;
                    }
                }
            } else {
                if (l3_miss_rate <= 0.003) {
                    return ExecutionPhase::DenseSequential;
                } else {
                    if (branch_miss_rate <= 0.035) {
                        return ExecutionPhase::DenseSequential;
                    } else {
                        return ExecutionPhase::PointerChasing;
                    }
                }
            }
        } else {
            if (ipc <= 1.044) {
                if (branch_miss_rate <= 0.052) {
                    return ExecutionPhase::SparseRandom;
                } else {
                    return ExecutionPhase::PointerChasing;
                }
            } else {
                if (l3_miss_rate <= 0.015) {
                    return ExecutionPhase::PointerChasing;
                } else {
                    return ExecutionPhase::SparseRandom;
                }
            }
        }

        return ExecutionPhase::Unknown;
    }
};

} // namespace morpheus

#endif // MORPHEUS_TRAINED_CLASSIFIER_H
