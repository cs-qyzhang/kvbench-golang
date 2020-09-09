#pragma once

#include <thread>
#include <vector>
#include <iomanip>

#include "../nlohmann/json.hpp"
#include "options.hpp"
#include "timer.hpp"

using nlohmann::json;

namespace kvbench {

thread_local int thread_id_ = 0;

// base class of database
class DB {
 public:
  virtual ~DB() {}

  virtual std::string Name() const = 0;

  virtual bool Get(void* key, void** value) = 0;
  virtual bool Put(void* key, void* value) = 0;
  virtual bool Update(void* key, void* value) = 0;
  virtual bool Delete(void* key) = 0;
  virtual size_t Scan(void* min_key, size_t max_size, std::vector<void*>* values) = 0;

  virtual void PhaseBegin(Phase::Type type, size_t size) {}
  virtual void PhaseEnd(Phase::Type type, size_t size) {}

  virtual int GetThreadNumber() const { return nr_thread_; };
  virtual void SetThreadNumber(int nr_thread) { nr_thread_ = nr_thread; }

  virtual int GetThreadId() const { return thread_id_; }
  virtual void SetThreadId(int thread_id) { thread_id_ = thread_id; }

  virtual void ThreadInit(int thread_id) {}

 private:
  int nr_thread_;
};

class Bench {
 public:
  Bench(int argc, char** argv) : opt_(argc, argv) {}

  void SetDB(DB* db) { db_ = db; }

  void Run() { Run_(); }

 private:
  std::string HumanReadableNumber_(double number) const {
    std::string res = std::to_string(number);

    auto pos = res.find('.');
    std::string int_part = res.substr(0, pos);

    res = int_part.substr(0, int_part.size() % 3);
    for (auto i = int_part.size() % 3; i < int_part.size(); i += 3)
      res += "," + int_part.substr(i, 3);
    if (res[0] == ',')
      res = res.substr(1);
    return res;
  }

  void PrintStats_() const {
    if (opt_.phases.size() <= 0) return;
    std::cout << std::endl
              << "============================== STATICS " "==============================" << std::endl
              << "DB name:            " << db_->Name() << std::endl
              << "Total run time (s): " << overall_stat_.duration / 1000000.0 << std::endl;

    for (size_t i = 0; i < opt_.phases.size(); ++i) {
      auto stat = opt_.phases[i].stat;
      std::cout << std::endl
                << "-------------------- PHASE " << i + 1 << ": " << opt_.phases[i].type << "--------------------" << std::endl
                << "  " << "Run time (s):         " << stat.duration / 1000000.0 << std::endl
                << "  " << "Total:                " << stat.total << std::endl
                << "  " << "Average latency (us): " << stat.average_latency << std::endl
                << "  " << "Maximum latency (us): " << stat.max_latency << std::endl
                << "  " << "Throughput (ops/s):   " << HumanReadableNumber_(stat.throughput) << std::endl;
    }

    std::cout << "============================ END STATICS " "============================" << std::endl;
  }

  void DumpStatsToJson_() {
    std::vector<Stat> stats;
    std::vector<std::string> name;
    stats.push_back(overall_stat_);
    name.push_back("overall");
    for (auto& phase : opt_.phases) {
      stats.push_back(phase.stat);
      name.push_back(ToString(phase.type));
    }
    json j = stats;
    for (size_t idx = 0; idx < j.size(); idx++) {
      j.at(idx)["name"] = name[idx];
    }

    try {
      std::ofstream file(opt_.output_file, std::ios::trunc | std::ios::out);
      file << j.dump(4) << std::endl;
    } catch (std::exception& e) {
      std::cerr << e.what() << std::endl;
    }
  }

  void Run_() {
    overall_stat_.duration = 0.0;
    for (auto& phase : opt_.phases) {
      phase.ReadData();
      overall_stat_.total += phase.run_size;

      db_->PhaseBegin(phase.type, phase.run_size);
      overall_stat_.duration += RunPhase_(phase);
      db_->PhaseEnd(phase.type, phase.run_size);
    }
    overall_stat_.CaculateStatistic();
    PrintStats_();
    DumpStatsToJson_();
  }

  void RunPhaseMain_(int thread_id, Phase& phase, size_t begin, size_t size,
                     double& total_latency, double& max_latency,
                     std::vector<double>& latencys, int sample_interval) {
    db_->SetThreadId(thread_id);
    db_->ThreadInit(thread_id);

    double latency;
    Timer latency_timer;
    max_latency = 0.0;
    total_latency = 0.0;

#define KVBENCH_RECORD_START \
  do {                       \
    latency_timer.Start();   \
  } while (0)

#define KVBENCH_RECORD_END                                     \
  do {                                                         \
    latency = latency_timer.End();                             \
    total_latency += latency;                                  \
    max_latency = std::max(max_latency, latency);              \
    if (i % sample_interval == 0) latencys.push_back(latency); \
  } while (0)

    if (phase.type == Phase::Type::LOAD || phase.type == Phase::Type::PUT) {
      for (size_t i = 0; i < size; ++i) {
        void* key = phase.key_reader->At(i + begin);
        void* value = phase.value_reader->At(i + begin);
        KVBENCH_RECORD_START;
        db_->Put(key, value);
        KVBENCH_RECORD_END;
      }
    } else if (phase.type == Phase::Type::GET) {
      for (size_t i = 0; i < size; ++i) {
        void* key = phase.key_reader->At(i + begin);
        void* value;
        KVBENCH_RECORD_START;
        db_->Get(key, &value);
        KVBENCH_RECORD_END;
      }
    } else if (phase.type == Phase::Type::UPDATE) {
      for (size_t i = 0; i < size; ++i) {
        void* key = phase.key_reader->At(i + begin);
        void* value = phase.value_reader->At(i + begin);
        KVBENCH_RECORD_START;
        db_->Update(key, value);
        KVBENCH_RECORD_END;
      }
    } else if (phase.type == Phase::Type::DELETE) {
      for (size_t i = 0; i < size; ++i) {
        void* key = phase.key_reader->At(i + begin);
        KVBENCH_RECORD_START;
        db_->Delete(key);
        KVBENCH_RECORD_END;
      }
    } else if (phase.type == Phase::Type::SCAN) {
      for (size_t i = 0; i < size; ++i) {
        void* min_key = phase.key_reader->At(i + begin);
        std::vector<void*> values;
        KVBENCH_RECORD_START;
        db_->Scan(min_key, phase.scan_size, &values);  // TODO
        KVBENCH_RECORD_END;
      }
    } else {
      assert(0);
    }

#undef KVBENCH_RECORD_START
#undef KVBENCH_RECORD_END
  }  // namespace kvbench

  double RunPhase_(Phase& phase) {
    phase.stat.total = phase.run_size;
    Timer timer;
    int sample_interval = phase.run_size < 2000000 ? 1 : phase.run_size / 2000000;

    std::vector<double> total_latency;
    std::vector<double> max_latency;
    std::vector<std::vector<double>> latencys;
    total_latency.resize(phase.nr_thread);
    max_latency.resize(phase.nr_thread);
    latencys.resize(phase.nr_thread);

    timer.Start();

    std::vector<std::thread> test_threads;
    size_t per_thread = phase.run_size / phase.nr_thread;
    for (size_t thread_id = 0; thread_id < phase.nr_thread; ++thread_id) {
      size_t size;
      if (thread_id != phase.nr_thread - 1)
        size = per_thread;
      else
        size = phase.run_size - (per_thread) * (phase.nr_thread - 1);
      test_threads.emplace_back(&Bench::RunPhaseMain_, this, thread_id,
                                std::ref(phase), thread_id * per_thread, size,
                                std::ref(total_latency[thread_id]),
                                std::ref(max_latency[thread_id]),
                                std::ref(latencys[thread_id]), sample_interval);
    }

    for (auto&& test_thread : test_threads)
      if (test_thread.joinable()) test_thread.join();

    phase.stat.duration = timer.End();

    for (size_t i = 0; i < phase.nr_thread; ++i)
      phase.stat.latency.insert(phase.stat.latency.end(), latencys[i].begin(),
                                latencys[i].end());

    double latency_sum =
        std::accumulate(total_latency.cbegin(), total_latency.cend(), 0.0);
    phase.stat.average_latency = latency_sum / phase.run_size;
    phase.stat.max_latency =
        *std::max_element(max_latency.cbegin(), max_latency.cend());
    phase.stat.CaculateStatistic();

    return phase.stat.duration;
  }

 private:
  Options opt_;
  DB* db_;
  Stat overall_stat_;
};

#define KVBENCH_MAIN(db)              \
  int main(int argc, char** argv) {   \
    kvbench::Bench bench{argc, argv}; \
    bench.SetDB(new db());            \
    bench.Run();                      \
    return 0;                         \
  }

}  // namespace kvbench