#ifndef MORPHEUS_TIMER_H
#define MORPHEUS_TIMER_H

#include <chrono>
#include <cstdint>

namespace morpheus {

class Timer {
public:
    Timer() : start_time_(std::chrono::high_resolution_clock::now()) {}
    
    void reset() {
        start_time_ = std::chrono::high_resolution_clock::now();
    }
    
    uint64_t elapsedNanoseconds() const {
        auto end_time = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            end_time - start_time_).count();
    }
    
    double elapsedMilliseconds() const {
        return elapsedNanoseconds() / 1e6;
    }
    
    double elapsedSeconds() const {
        return elapsedNanoseconds() / 1e9;
    }

private:
    std::chrono::high_resolution_clock::time_point start_time_;
};

} // namespace morpheus

#endif // MORPHEUS_TIMER_H