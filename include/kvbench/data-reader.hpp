#pragma once

#include <fstream>
#include <vector>
#include "random.hpp"

namespace kvbench {

class DataReader {
 public:
  DataReader() : max_size_(0), rnd_(nullptr) {}

  bool Read(std::string file_name, size_t max_size) {
    max_size_ = max_size;
    if (file_name.empty()) {
      rnd_ = new RandomUniformUint64();
      return true;
    }

    std::ifstream file(file_name);
    if (file.fail()) {
      std::cerr << "CAN'T OPEN " << file_name << " FILE! EXIT." << std::endl;
      exit(0);
    }
    uint64_t next;
    while (data_.size() < max_size && (file >> next))
      data_.push_back(next);
    return true;
  }

  std::vector<uint64_t>& Data() {
    return data_;
  }

  size_t Size() const {
    return data_.size();
  }

  inline void* At(size_t index) const {
    uint64_t data;
    if (rnd_ != nullptr)
      rnd_->Next(reinterpret_cast<void*>(&data));
    else
      data = data_.at(index);
    return reinterpret_cast<void*>(data);
  }

 private:
  size_t max_size_;
  std::vector<uint64_t> data_;
  RandomUniformUint64* rnd_;
};

}