#pragma once

#include <cstddef>
#include <cstring>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <iterator>
#include <sstream>
#include "stat.hpp"
#include "random.hpp"
#include "data-reader.hpp"

namespace kvbench {

struct Phase {
  enum class Type {
    NONE,
    LOAD,
    PUT,
    GET,
    UPDATE,
    DELETE,
    SCAN,
  };

  Type type;
  size_t nr_thread;
  size_t run_size;
  size_t scan_size;       // only needed in SCAN
  std::string key_type;   // e.g. "uint64_t", "std::string"...
  std::string value_type;
  std::string key_data_file;
  std::string value_data_file;
  DataReader* key_reader;
  DataReader* value_reader;
  Stat stat;

  Phase() : type(Type::NONE), nr_thread(1), run_size(0), scan_size(0),
            key_type("uint64_t"), value_type("uint64_t"),
            key_data_file(""), value_data_file("") {}

  bool Validate() const {
    return true;
  }

  bool ReadData() {
    key_reader = new DataReader();
    key_reader->Read(key_data_file, run_size);
    if (key_reader->Size() < run_size)
      return false;
    if (value_data_file == key_data_file) {
      value_reader = key_reader;
    } else {
      value_reader = new DataReader();
      value_reader->Read(value_data_file, run_size);
      if (value_reader->Size() < run_size)
        return false;
    }
    return true;
  }

  void Clear(size_t threads) {
    type            = Type::NONE;
    nr_thread       = threads;
    run_size        = 0;
    scan_size       = 0;
    key_type        = "uint64_t";
    value_type      = "uint64_t";
    key_data_file   = "";
    value_data_file = "";
  }
};

std::string ToString(Phase::Type type) {
  switch (type) {
    case Phase::Type::NONE:   return "none";
    case Phase::Type::LOAD:   return "load";
    case Phase::Type::PUT:    return "put";
    case Phase::Type::GET:    return "get";
    case Phase::Type::UPDATE: return "update";
    case Phase::Type::DELETE: return "delete";
    case Phase::Type::SCAN:   return "scan";
    default:                  return "ERROR";
  }
}

std::ostream& operator<<(std::ostream& os, const Phase::Type& type) {
  os << ToString(type);
  return os;
}

struct Options {
  enum class Type {
    PHASE,
    YCSB,
  };

  Type bench_type;
  std::vector<Phase> phases;
  std::string prog_name;
  std::string output_file;  // stat.json

  Options(int argc, char** argv)
    : prog_name("???"), output_file("stat.json")
  {
    ParseOptions(argc, argv);
  }

  [[ noreturn ]] void ShowHelp() {
    std::cout << "Usage: " << prog_name << " [OPTION] PHASES" << std::endl
              << std::endl
              << "  OPTION:" << std::endl
              << "    -t,--thread <number>                          default run thread number" << std::endl
              << "    --are-you-kvbench                             print \"YES!\" and exit" << std::endl
              << "    -h,--help                                     display this help and exit" << std::endl
              << std::endl
              << "  PHASES:" << std::endl
              << "    load[=<thread-number>] <run-size>             specify load phase" << std::endl
              << "    put[=<thread-number>] <run-size>              specify put phase" << std::endl
              << "    get[=<thread-number>] <run-size>              specify get phase" << std::endl
              << "    update[=<thread-number>] <run-size>           specify update phase" << std::endl
              << "    delete[=<thread-number>] <run-size>           specify delete phase" << std::endl
              << "    scan<scan_size>[=<thread-number>] <run-size>  specify scan phase" << std::endl
              << std::endl
              << "e.g.: " << prog_name << " -t 8 load 1000000 get=1 10000 get=2 10000 scan100=4 10000 delete 10000" << std::endl
              << std::endl
              << "  above command first load 100000 data using 8 threads, then get 10000 data using" << std::endl
              << "  1 thread, get 10000 data using 2 threads, scan 100 items for 1000 times using" << std::endl
              << "  4 threads, at last delete 10000 data using 8 threads." << std::endl
              ;
    exit(0);
  }

  [[ noreturn ]] void AreYouKVBench() {
    std::cout << "YES!" << std::endl;
    exit(0);
  }

  bool GetUint64(std::string str, uint64_t& res) {
    try {
      res = std::stoull(str);
    } catch (const std::invalid_argument& e) {
      return false;
    } catch (const std::out_of_range& e) {
      return false;
    }
    return true;
  }

  bool ArgEqual(std::string arg, std::initializer_list<std::string>&& expected) {
    for (auto& e : expected)
      if (arg == e)
        return true;
    return false;
  }

  [[ noreturn ]] void ParseError(std::string reason) {
    std::cerr << reason << std::endl << std::endl;
    ShowHelp();
  }

  bool ParsePhaseArg(std::string arg1, std::string arg2, Phase& phase) {
    // not case sensitive
    // FIXME: hard-coded 6
    for (int i = 0; i < std::min<int>(arg1.length(), 6); ++i) {
      arg1[i] = std::tolower(arg1[i]);
    }

    if (arg1.substr(0, 4) == "load") {
      phase.type = Phase::Type::LOAD;
      arg1 = arg1.substr(4);
    } else if (arg1.substr(0, 3) == "put") {
      phase.type = Phase::Type::PUT;
      arg1 = arg1.substr(3);
    } else if (arg1.substr(0, 3) == "get") {
      phase.type = Phase::Type::GET;
      arg1 = arg1.substr(3);
    } else if (arg1.substr(0, 6) == "update") {
      phase.type = Phase::Type::UPDATE;
      arg1 = arg1.substr(6);
    } else if (arg1.substr(0, 6) == "delete") {
      phase.type = Phase::Type::DELETE;
      arg1 = arg1.substr(6);
    } else if (arg1.substr(0, 4) == "scan") {
      phase.type = Phase::Type::SCAN;
      arg1 = arg1.substr(4);
    } else {
      return false;
    }

    // get scan size
    if (phase.type == Phase::Type::SCAN) {
      auto pos = arg1.find('=');
      std::string scan_size_str = arg1.substr(0, pos);
      if (!GetUint64(scan_size_str, phase.scan_size))
        ParseError("Wrong scan size!");
      arg1 = (pos == std::string::npos) ? "" : arg1.substr(pos);
    }

    if (arg1.size() > 0) {
      if (arg1[0] != '=')
        ParseError("Wrong format!");
      arg1.erase(0, 1);

      // get specified thread number
      auto pos = arg1.find(',');
      std::string nr_thread_str = arg1.substr(0, pos);
      if (nr_thread_str != "" && !GetUint64(nr_thread_str, phase.nr_thread))
        ParseError("Wrong thread number!");
      arg1 = (pos == std::string::npos) ? "" : arg1.substr(pos + 1);

      // get specified key type
      pos = arg1.find(',');
      phase.key_type = (arg1.substr(0, pos) != "") ? arg1.substr(0, pos) : phase.key_type;
      arg1 = (pos == std::string::npos) ? "" : arg1.substr(pos + 1);

      // get specified value type
      pos = arg1.find(',');
      phase.value_type = (arg1.substr(0, pos) != "") ? arg1.substr(0, pos) : phase.value_type;
      arg1 = (pos == std::string::npos) ? "" : arg1.substr(pos + 1);

      // get key data file
      pos = arg1.find(',');
      phase.key_data_file = (arg1.substr(0, pos) != "") ? arg1.substr(0, pos) : "";
      arg1 = (pos == std::string::npos) ? "" : arg1.substr(pos + 1);

      // get value data file
      phase.value_data_file = arg1;

      // validate
      phase.Validate();
    }

    // get run size
    if (!GetUint64(arg2, phase.run_size))
      ParseError("Wrong run size!");
    return true;
  }

  void ParseOptions(int argc, char** argv) {
    prog_name = argv[0];

    int i = 1;
    Phase phase;
    size_t nr_thread = 1;
    while (i < argc) {
      if (ArgEqual(argv[i], {"--are-you-kvbench"})) {
        // no return
        AreYouKVBench();
      } else if (ArgEqual(argv[i], {"-h", "--help"})) {
        // no return
        ShowHelp();
      } else if (ArgEqual(argv[i], {"-o", "--output"})) {
        if (i + 1 >= argc)
          ParseError(std::string(argv[i]) + " option needs a file name!");
        output_file = argv[i + 1];
        i += 2;
      } else if (ArgEqual(argv[i], {"-t", "--thread"})) {
        if (i + 1 >= argc || !GetUint64(argv[i + 1], nr_thread))
          ParseError(std::string(argv[i]) + " option needs an demical number!");
        phase.nr_thread = nr_thread;
        i += 2;
      } else if (i + 1 < argc && ParsePhaseArg(argv[i], argv[i + 1], phase)) {
        phases.push_back(phase);
        phase.Clear(nr_thread);
        i += 2;
      } else {
        ParseError("Unknown argument " + std::string(argv[i]) + "!");
      }
    }
    if (phases.empty())
      ParseError("No phase specified!");
    std::cout << ToString() << std::endl;
  }

  std::string ToString() const {
    std::string res = "";
    for (auto& phase : phases) {
      std::stringstream ss;
      ss << phase.type;
      if (phase.type == Phase::Type::SCAN)
        ss << phase.scan_size;
      ss << " " << phase.run_size << " in " << phase.nr_thread
         << "-thread. "
         << "key type: " << phase.key_type << ", "
         << "value type: " << phase.value_type << ", "
         << "key data file: " << phase.key_data_file << ", "
         << "value data file: " << phase.value_data_file << ".";
      res += ss.str();
    }
    return res;
  }
};

} // namespace kvbench