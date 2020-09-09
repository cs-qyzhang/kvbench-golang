#include <iostream>
#include "kvbench/kvbench.hpp"
#include "third-party/FAST_FAIR/btree.h"

using namespace fastfair;

class FastFair : public kvbench::DB {
 public:
  FastFair() : db_(new btree()) {}
  ~FastFair() { delete db_; }

  bool Get(void* key, void** value) {
    db_->btree_search((uint64_t)key);
    return 0;
  }

  bool Put(void* key, void* value) {
    db_->btree_insert((uint64_t)key, (char *)value);
    return 0;
  }

  bool Update(void* key,  void* value) {
    return 0;
  }

  bool Delete(void* key) {
    db_->btree_delete((uint64_t)key);
    return 0;
  }

  size_t Scan(void* min_key, size_t max_size, std::vector<void*>* values) {
    int results_found = 0;
    uint64_t results[max_size];
    db_->btree_search_range ((uint64_t)min_key, UINT64_MAX, results, max_size, results_found);
    delete results;
    return 0;
  }

  std::string Name() const {
    return "Fast Fair";
  }

 private:
  btree* db_;
};

KVBENCH_MAIN(FastFair)
