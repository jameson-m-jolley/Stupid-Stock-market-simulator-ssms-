#pragma once

#include "util/logger.hpp"
#include "nlohmann/json.hpp"
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits.h>
#include <sstream>
#include <string>
#include <unistd.h>
using json = nlohmann::json;

class config {
private:
  config();

  config(const config &) = delete;
  config &operator=(const config &) = delete;
  config(config &&) = delete;
  config &operator=(config &&) = delete;
  bool is_valid();
  bool clamp_valid_vals();

public:
  using clock = std::chrono::steady_clock;
  using duration = std::chrono::milliseconds;
  clock::time_point start_time;
  const char *file_path;
  const char *program_name;
  unsigned short mode;
  static const unsigned int MAX_SIZE = 1000000000;
  static const unsigned int MIN_SIZE = 1;
  unsigned int size;
  bool debug;
  unsigned int seed;
  float update_time;
  bool running;
  bool paused;
  unsigned int port;
  duration runtime;
  bool running_forever;
  bool clamp_config;
  bool unsafe;
  bool save();
  bool load();
  bool load_args(int argc, char *argv[]);
  void print_help();
  json config_to_json();
  size_t num_stocks;
  int num_traders;
  int stock_price;
  int initial_wallet;
  int seed_orders;
  bool should_terminate;
  int monte_carlo_runs;
  char *monty_carlo_file_save_path;

  const char *host = "localhost";

  static config &instance();
};