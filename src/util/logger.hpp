#pragma once
#include <memory>
#include <spdlog/spdlog.h>

class Logger {
public:
  static void init(bool json_mode = false,
                   spdlog::level::level_enum level = spdlog::level::info);
  static std::shared_ptr<spdlog::logger> &get();
  static void set_level(spdlog::level::level_enum level);

private:
  static std::shared_ptr<spdlog::logger> logger;
};
