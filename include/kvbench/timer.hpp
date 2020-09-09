#pragma once

#include <chrono>

namespace kvbench {

// microseconds timer
class Timer {
 public:
  void Start() { start_ = std::chrono::high_resolution_clock::now(); }

  double End() {
    end_ = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end_ - start_;
    return duration.count() * 1000000;
  }

 private:
  std::chrono::time_point<std::chrono::high_resolution_clock> start_;
  std::chrono::time_point<std::chrono::high_resolution_clock> end_;
};

} // namespace kvbench