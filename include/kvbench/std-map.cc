#include <map>
#include "kvbench.hpp"

using namespace kvbench;

class StdMap : public DB {
 public:
  bool Get(void* key, void** value) override {
    uint64_t ikey = reinterpret_cast<uint64_t>(key);
    if (internal_.count(ikey)) {
      *(reinterpret_cast<uint64_t*>(value)) = internal_[ikey];
      return true;
    }
    return false;
  }

  bool Put(void* key, void* value) override {
    uint64_t ikey = reinterpret_cast<uint64_t>(key);
    uint64_t ival = reinterpret_cast<uint64_t>(value);
    internal_[ikey] = ival;
    return true;
  }

  bool Update(void* key, void* value) override {
    uint64_t ikey = reinterpret_cast<uint64_t>(key);
    uint64_t ival = reinterpret_cast<uint64_t>(value);
    internal_[ikey] = ival;
    return true;
  }

  bool Delete(void* key) override {
    uint64_t ikey = reinterpret_cast<uint64_t>(key);
    internal_.erase(ikey);
    return true;
  }

  size_t Scan(void* min_key, std::vector<void*>* values) override {
    std::cout << GetThreadId() << std::endl;
    return 0;
  }

  std::string Name() const {
    return "std::map";
  }

 private:
  std::map<uint64_t,uint64_t> internal_;
};

KVBENCH_MAIN(StdMap)