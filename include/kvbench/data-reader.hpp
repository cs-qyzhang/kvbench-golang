#pragma once

#include <fstream>
#include <vector>
#include <iostream>
#include "random.hpp"

namespace kvbench {

class DataReader {
 public:
  DataReader() : max_size_(0) {}

  bool Read(std::string file_name, size_t max_size) {
    max_size_ = max_size;
    if (file_name.empty()) {
      RandomUniformUint64 rnd;
      while (data_.size() < max_size) {
        uint64_t num;
        rnd.Next(reinterpret_cast<void*>(&num));
        data_.push_back(num);
      }
      return true;
    }

    std::ifstream file(file_name);
    if (file.fail()) {
      std::cerr << "CAN'T OPEN " << file_name << " FILE! EXIT." << std::endl;
      exit(0);
    }
    uint64_t next;
    std::cout << "start reading data file: " << file_name << std::endl;
    while (data_.size() < max_size && (file >> next))
      data_.push_back(next);
    std::cout << "finish reading data file: " << file_name << std::endl;
    return true;
  }

  size_t Size() const {
    return data_.size();
  }

  inline void* At(size_t index) const {
    return reinterpret_cast<void*>(data_.at(index));
  }

 private:
  size_t max_size_;
  std::vector<uint64_t> data_;
};

}