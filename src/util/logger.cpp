#include "util/logger.hpp"
#include "util/config.hpp"
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <vector>

std::shared_ptr<spdlog::logger> Logger::logger;

void Logger::init(bool json_mode, spdlog::level::level_enum level) {
  std::vector<spdlog::sink_ptr> sinks;

  auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console->set_pattern("[%T] [%^%l%$] %v");
  sinks.push_back(console);

  std::string hostStr = config::instance().host;
  std::string portStr = std::to_string(config::instance().port);
  std::string filePath = "logs/app-" + hostStr + "-" + portStr + ".log";


  auto file = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filePath, 5 * 1024 * 1024, 3);
  if (json_mode)
    file->set_pattern(R"({"time":"%T","level":"%l","msg":"%v"})");
  else
    file->set_pattern("[%T] [%l] %v");
  sinks.push_back(file);

  logger = std::make_shared<spdlog::logger>("multi_sink", sinks.begin(),
                                            sinks.end());
  spdlog::register_logger(logger);
  logger->set_level(level);
  logger->flush_on(level);
}

std::shared_ptr<spdlog::logger> &Logger::get() { return logger; }

void Logger::set_level(spdlog::level::level_enum level) {
  if (logger)
    logger->set_level(level);
}
