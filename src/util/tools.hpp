#pragma once

#include "util/config.hpp"
#include "util/logger.hpp"
#include <cstdio> // Required for printf
#include <string>
// this is a header for utilities mainly macros, new inline function also used
// to clean up usage.

// Define a macro that lists all the required headers.
// Each header is on a new line and ends with a backslash for continuation.
#define MAKE_INCLUSIVE_RANGE_TEST(DEBUG, LOW, HIGH, VAL, LOCATION)             \
  if (DEBUG) {                                                                 \
    if (HIGH < VAL || VAL < LOW) {                                             \
      throw std::out_of_range(                                                 \
          "\n\ta value was passed into a function could cause undefined\n"     \
          "\tbehavior in " LOCATION);                                          \
    };                                                                         \
  };

#define MAKE_EXCLUSIVE_RANGE_TEST(DEBUG, LOW, HIGH, VAL, LOCATION)             \
  if (DEBUG) {                                                                 \
    if (HIGH <= VAL || VAL <= LOW) {                                           \
      throw std::out_of_range(                                                 \
          "\n\ta value was passed into a function could cause undefined\n"     \
          "\tbehavior in " LOCATION);                                          \
    };                                                                         \
  };

// Helper macro for stringify
#define STRINGIFY(x) #x
// Macro to create a JSON key-value string
#define MAKE_JSON_STRING(KEY, VALUE)                                           \
  "\"" STRINGIFY(KEY) "\":\"" STRINGIFY(VALUE) "\""
#define MAKE_JSON_STRING_WITH_VAL(KEY, VALUE)                                  \
  "\"" STRINGIFY(KEY) "\":\"" STRINGIFY(VALUE) "\""

void print_JSON_key_val(char *key, char *value, FILE *out);
std::string make_json_string(std::string key, std::string val);

inline void init_config_and_logger(int argc, char *argv[]) {
  config &conf = config::instance();
  bool json_logs = std::getenv("LOG_JSON") != nullptr;
  std::string lvl =
      std::getenv("LOG_LEVEL") ? std::getenv("LOG_LEVEL") : "info";
  spdlog::level::level_enum level = spdlog::level::info;
  if (lvl == "debug")
    level = spdlog::level::debug;
  else if (lvl == "warn")
    level = spdlog::level::warn;
  else if (lvl == "error")
    level = spdlog::level::err;

  Logger::init(json_logs, level);
  Logger::get()->info("app: Stock Market Simulator starting up (argc={})",
                      argc);

  if (conf.load_args(argc, argv)) {
    if (conf.debug) {
      Logger::get()->debug("app: command line args were loaded successfully");
    }
  } else {
    Logger::get()->error("app: loading command line args failed");
  }

  if (conf.mode == 1) {
    conf.running = 1;
  }
}