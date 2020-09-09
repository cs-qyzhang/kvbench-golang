#pragma once

#include <random>
#include <chrono>

namespace kvbench {

class Random {
 public:
  virtual ~Random() {}

  virtual void Next(void*) = 0;
};

class RandomUniformInt : public Random {
 public:
  RandomUniformInt() : dist_() {
    // https://stackoverflow.com/a/13446015/7640227
    std::random_device rd;
    // seed value is designed specifically to make initialization
    // parameters of std::mt19937 (instance of std::mersenne_twister_engine<>)
    // different across executions of application
    std::mt19937::result_type seed = rd() ^ (
            (std::mt19937::result_type)
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
                ).count() +
            (std::mt19937::result_type)
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()
                ).count() );
    gen_.seed(seed);
  }

  void Next(void* ptr) {
    *reinterpret_cast<int*>(ptr) = dist_(gen_);
  }

 private:
  std::mt19937 gen_;
  std::uniform_int_distribution<int> dist_;
};

class RandomUniformUint64 : public Random {
 public:
  RandomUniformUint64() : dist_() {
    // https://stackoverflow.com/a/13446015/7640227
    std::random_device rd;
    // seed value is designed specifically to make initialization
    // parameters of std::mt19937 (instance of std::mersenne_twister_engine<>)
    // different across executions of application
    std::mt19937_64::result_type seed = rd() ^ (
            (std::mt19937_64::result_type)
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
                ).count() +
            (std::mt19937_64::result_type)
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()
                ).count() );
    gen_.seed(seed);
  }

  void Next(void* ptr) {
    *(uint64_t*)ptr = dist_(gen_);
  }

 private:
  std::mt19937_64 gen_;
  std::uniform_int_distribution<uint64_t> dist_;
};

} // namespace kvbench