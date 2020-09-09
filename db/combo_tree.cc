#include <iostream>
#include <filesystem>
#include "../include/kvbench/kvbench.hpp"
#include "combotree/combotree.h"

#define COMBO_TREE_DIR  "/mnt/pmem0/combotree/"

class ComboTree : public kvbench::DB {
 public:
  ComboTree() {
    std::filesystem::remove_all(COMBO_TREE_DIR);
    std::filesystem::create_directory(COMBO_TREE_DIR);
    db_ = new combotree::ComboTree(COMBO_TREE_DIR, PMEMOBJ_MIN_POOL * 100);
  }

  ~ComboTree() {
    delete db_;
  }

  bool Get(void* key, void** value) {
    uint64_t val;
    db_->Get(reinterpret_cast<uint64_t>(key), val);
    *(uint64_t*)value = val;
    return true;
  }

  bool Put(void* key, void* value) {
    db_->Insert(reinterpret_cast<uint64_t>(key), reinterpret_cast<uint64_t>(value) & 0x3FFFFFFFFFFFFFFFUL);
    return true;
  }

  bool Update(void* key,  void* value) {
    return true;
  }

  bool Delete(void* key) {
    db_->Delete(reinterpret_cast<uint64_t>(key));
    return true;
  }

  size_t Scan(void* min_key, size_t max_size, std::vector<void*>* values) {
    uint64_t results[max_size];
    size_t cnt = db_->Scan(reinterpret_cast<uint64_t>(min_key), UINT64_MAX, max_size, results);
    assert(cnt <= max_size);
    return cnt;
  }

  void PhaseEnd(kvbench::Phase::Type type, size_t size) {
    if (type == kvbench::Phase::Type::LOAD) {
      while (db_->IsExpanding())
        std::this_thread::sleep_for(std::chrono::seconds(1));
      std::cout << "load phase end" << std::endl;
    }
  }

  std::string Name() const {
    return "Combo Tree";
  }

 private:
  combotree::ComboTree* db_;
};

KVBENCH_MAIN(ComboTree)
