#pragma once

#include <vector>
#include <cstdint>
#include "../nlohmann/json.hpp"

namespace kvbench {

using nlohmann::json;

struct Stat {
  std::vector<double> latency;
  double duration;
  uint64_t failed;
  uint64_t total;

  // caculated
  double throughput;
  double max_latency;
  double average_latency;

  Stat() : duration(0.0), failed(0), total(0), throughput(0.0),
           max_latency(0.0), average_latency(0.0) {}

  void AddLatency(double new_latency) {
    latency.push_back(new_latency);
  }

  void CaculateStatistic() {
    // TODO
    throughput = total / duration * 1000000.0;
  }
};

void to_json(json& j, const Stat& stat) {
    j = json{{"duration", stat.duration},
             {"total", stat.total},
             {"throughput", stat.throughput},
             {"maxLatency", stat.max_latency},
             {"averageLatency", stat.average_latency}};
}

} // namespace kvbench