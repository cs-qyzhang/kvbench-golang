#include <map>
#include "../include/kvbench/kvbench.hpp"

class StdMap : public kvbench::DB {
 public:
  std::string Name() const override {
    return "std::map";
  }

  bool Get(void* key, void** value) override {
    auto iter = db_.find(reinterpret_cast<uint64_t>(key));
    if (iter == db_.end())
      return false;
    else
      *value = reinterpret_cast<void*>(iter->second);
    return true;
  }

  bool Put(void* key, void* value) override {
    db_.insert(std::pair<uint64_t, uint64_t>((uint64_t)key, (uint64_t)value));
    return true;
  }

  bool Update(void* key, void* value) override {
    return true;
  }

  bool Delete(void* key) override {
    db_.erase((uint64_t)key);
    return true;
  }

  size_t Scan(void* min_key, size_t max_size, std::vector<void*>* values) override {
    return 0;
  }

 private:
  std::map<uint64_t, uint64_t> db_;

};

KVBENCH_MAIN(StdMap)
