#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h> // Required for sleep() in seconds
#include <vector>

#include "sim.hpp"
#include "net/socket.hpp"
#include "util/tools.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  init_config_and_logger(argc, argv);

  config &conf = config::instance();
  sim &app = sim::instance();
  socket &server = socket::instance();

  if (conf.seed == 0) {
    conf.seed = std::random_device{}();
    Logger::get()->debug("app: using generated random seed: {}", conf.seed);
  } else {
    Logger::get()->debug("app: using loaded random seed: {}", conf.seed);
  }

  if (conf.mode == 2) {
    if (server.start()) {
      std::string url =
          "./frontEnd/index.html"; // Replace with your desired URL
      std::string command;
      Logger::get()->info("app: websocket started");
#ifdef _WIN32 // For Windows
      command = "start " + url;
#elif __APPLE__ // For macOS
      command = "open " + url;
#else           // For Linux/Unix-like systems
      command = "xdg-open " +
                url; // Or use a specific browser command like "firefox " + url
#endif
      system(command.c_str());
    } else {
      Logger::get()->error("app: websocket failed to start");
    }
  }

  if (conf.mode == 3) {
    Logger::get()->info("app: running in monte carlo mode");
    // run tests here
    // we need to fork processes and run multiple sims
    // we need more config options for monte carlo
    // we need to save results to a file preferably csv
    Logger::get()->info("app: monte carlo mode not yet implemented");
    if (conf.monte_carlo_runs <= 0) {
      Logger::get()->error("app: monte carlo runs not set or invalid");
      return 1;
    }
    if (strcasecmp(conf.monty_carlo_file_save_path, "") == 0) {
      Logger::get()->error("app: monte carlo file save path not set");
      return 1;
    }
    for (int i = 0; i < conf.monte_carlo_runs; i++) {
      Logger::get()->info("app: monte carlo run {}/{}", i + 1,
                          conf.monte_carlo_runs);
    }
    return 0;
  }

  while (!conf.should_terminate) {
    if (conf.running == false) {
      sleep(1); // stop insane amount of continues
      continue;
    }

    if (app.run_sim()) {
      Logger::get()->info("sim: ended gracefully");
      if (conf.mode != 2) {
        conf.should_terminate = true;
      }

      continue;
    }

    Logger::get()->error("sim: CRITICAL undefined behaviour encountered");
  }

  Logger::get()->info("app: exiting normally");
  return 0;
}