#include "util/config.hpp"

config::config() {
  start_time = std::chrono::steady_clock::now();
  mode = 0;
  debug = false;
  seed = 0;
  update_time = 1000;
  running = 1;
  running_forever = true;
  unsafe = false;
  paused = false;
  num_traders = 10;
  num_stocks = 5;
  stock_price = 1000;   // cents
  initial_wallet = 500; // dollars
  seed_orders = 5;      // # of mock orders for seeding stocks
  should_terminate =
      false; // private sim-only needed flag, not for argument use! Should STAY
             // as false upon instantiation of config
}

config &config::instance() {
  static config conf;
  return conf;
}

// this tries to save to an xml or json file
// it will return a true if successful and a false if not
// check file path validity before writing
bool config::save() {
  if (!file_path || std::string(file_path).empty()) {
    Logger::get()->error(
        "app:[config] No file path specified for saving. Creating default.");
    file_path = "config.json";
  }

  //
  // ensure file saves in current directory if path lacks "./"
  std::string fixed_path = std::string(file_path);
  if (fixed_path.rfind("./", 0) != 0) {
    fixed_path = "./" + fixed_path;
  }

  // show absolute path using getcwd()
  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd)) != nullptr) {
    Logger::get()->debug("app:[config] Attempting to save config to: {}/{}",
                         cwd, fixed_path.substr(2));
  }

  std::ofstream outputFile(fixed_path);
  // make sure file opened correctly
  if (!outputFile.is_open()) {
    Logger::get()->error("app:[config] Failed to open file for saving: {}",
                         fixed_path);
    return false;
  }

  // write config to file as JSON-like text
  json contents = config_to_json();
  outputFile << contents.dump().c_str();
  outputFile.close();

  Logger::get()->info("app:[config] Configuration saved to {}", fixed_path);
  return true;
}

// TODO: make this work
json config::config_to_json() {
  json j;

  j["file_path"] = file_path ? std::string(file_path) : "";
  j["program_name"] = program_name ? std::string(program_name) : "";
  j["mode"] = mode;
  j["size"] = size;
  j["debug"] = debug;
  j["seed"] = seed;
  j["update_time"] = update_time;
  j["port"] = port;
  j["running_forever"] = running_forever;
  j["clamp_config"] = clamp_config;
  j["unsafe"] = unsafe;
  j["num_stocks"] = num_stocks;
  j["num_traders"] = num_traders;
  j["stock_price"] = stock_price;
  j["initial_wallet"] = initial_wallet;
  j["seed_orders"] = seed_orders;

  return j;
}

// this tries to load from an xml or json file
// it will return a true if successful and a false if not
bool config::load() {
  if (!file_path || std::string(file_path).empty()) {
    Logger::get()->error("app:[config] No file path provided to load.");
    return false;
  }

  std::string fixed_path = std::string(file_path);
  if (fixed_path.rfind("./", 0) != 0) {
    fixed_path = "./" + fixed_path;
  }

  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd)) != nullptr) {
    Logger::get()->debug("app:[config] Attempting to load config from: {}/{}",
                         cwd, fixed_path.substr(2));
  }

  std::ifstream inputFile(fixed_path);
  if (!inputFile.is_open()) {
    Logger::get()->warn("app:[config] File not found. Will create new config.");
    return false;
  }

  try {
    json j;
    inputFile >> j;

    if (j.contains("mode") && j["mode"].is_number()) {
      mode = j["mode"];
      running = 0;
    }
    if (j.contains("size") && j["size"].is_number())
      size = j["size"];
    if (j.contains("debug") && j["debug"].is_boolean())
      debug = j["debug"];
    if (j.contains("seed") && j["seed"].is_number())
      seed = j["seed"];
    if (j.contains("update_time") && j["update_time"].is_number())
      update_time = j["update_time"];
    if (j.contains("port") && j["port"].is_number()) {
      long long p = j["port"];
      port = (p > 65535 || p < 0) ? 443 : static_cast<int>(p);
    }
    if (j.contains("running_forever") && j["running_forever"].is_boolean())
      running_forever = j["running_forever"];
    if (j.contains("clamp_config") && j["clamp_config"].is_boolean())
      clamp_config = j["clamp_config"];
    if (j.contains("unsafe") && j["unsafe"].is_boolean())
      unsafe = j["unsafe"];
    if (j.contains("num_stocks") && j["num_stocks"].is_number()) {
      num_stocks = j["num_stocks"];
    }
    if (j.contains("num_traders") && j["num_traders"].is_number()) {
      num_traders = j["num_traders"];
    }
    if (j.contains("stock_price") && j["stock_price"].is_number()) {
      stock_price = j["stock_price"];
    }
    if (j.contains("initial_wallet") && j["initial_wallet"].is_number()) {
      initial_wallet = j["initial_wallet"];
    }
    if (j.contains("seed_orders") && j["seed_orders"].is_number()) {
      seed_orders = j["seed_orders"];
    }

    Logger::get()->info("app:[config] Successfully loaded configuration.");
    return true;

  } catch (const std::exception &e) {
    Logger::get()->error("app:[config] Failed to parse JSON: {}", e.what());
    return false;
  }
}

bool config::is_valid() { return (bool)(false); }

bool config::clamp_valid_vals() { return (bool)(false); }

void config::print_help() {
  // keep original help text, just route via logger
  Logger::get()->info(
      "sim [options]\n"
      "options;\n"
      "-m\tmode\n"
      "\toption [1 = text based output, 2 = websocket mode, 3 = monty "
      "carlo mode]\n"
      "-d\tdebug infomation output\n"
      "-u\tupdate speed\n"
      "\toptions[0 = fastest possible, 1 = default]\n"
      "-s\tsize this sets the size of the sim; default 100\n"
      "-t\tsets the runtime (ms); default -1 = indefinite\n"
      "-p\tsets the port; default 443 (mode 2 only)\n"
      "-f\tpath to JSON config file (creates one if missing)\n"
      "-c\tclamp invalid values\n"
      "-unsafe\tignore runtime checks\n"
      "-seed\tsets the random seed\n"
      "-help\tprints help and exits\n"
      "-tr\tsets number of traders\n"
      "-st\tsets number of stocks\n"
      "-sp\tsets initial stock price (cents)\n"
      "-tw\tsets initial wallet (dollars)\n"
      "-so\tsets number of seed orders\n"
      "-load\tloads config from file path\n"
      "-mcr\tsets number of monte carlo runs (mode 3 only and -t != -1)\n"
      "-mcfsp\tsets monte carlo file save path (mode 3 only) \n");
}

/*loading the command line args*/
bool config::load_args(int argc, char *argv[]) {
  program_name = argv[0];
  int i = 1;
  while (i < argc) {
    if (strcmp(argv[i], "-m") == 0) {
      mode = std::atoi(argv[i + 1]);
      running = 0;
      i += 2;
    } else if (strcmp(argv[i], "-d") == 0) {
      debug = true;
      i++;
    } else if (strcmp(argv[i], "-u") == 0) {
      update_time = std::atoi(argv[i + 1]);
      i += 2;
    } else if (strcmp(argv[i], "-s") == 0) {
      size = std::atoi(argv[i + 1]);
      i += 2;
    } else if (strcmp(argv[i], "-t") == 0) {
      int num = std::atoi(argv[i + 1]);
      runtime = std::chrono::milliseconds(num);
      running_forever = (num <= -1);
      i += 2;
    } else if (strcmp(argv[i], "-p") == 0) {
      port = std::atoi(argv[i + 1]);
      i += 2;
    } else if (strcmp(argv[i], "-f") == 0) {
      file_path = argv[i + 1];
      i += 2;
    } else if (strcmp(argv[i], "-c") == 0) {
      clamp_config = true;
      i++;
    } else if (strcmp(argv[i], "-unsafe") == 0) {
      unsafe = true;
      i++;
    } else if (strcmp(argv[i], "-seed") == 0) {
      seed = std::atoi(argv[i + 1]);
      i += 2;
    } else if (strcmp(argv[i], "-tr") == 0) {
      num_traders = std::atoi(argv[i + 1]);
      i += 2;
    } else if (strcmp(argv[i], "-st") == 0) {
      num_stocks = std::atoi(argv[i + 1]);
      i += 2;
    } else if (strcmp(argv[i], "-sp") == 0) {
      stock_price = std::atoi(argv[i + 1]);
      i += 2;
    } else if (strcmp(argv[i], "-tw") == 0) {
      initial_wallet = std::atoi(argv[i + 1]);
      i += 2;
    } else if (strcmp(argv[i], "-so") == 0) {
      seed_orders = std::atoi(argv[i + 1]);
      i += 2;
    } else if (strcmp(argv[i], "-host") == 0) {
      host = argv[i + 1];
      i += 2;
    } else if (strcmp(argv[i], "-load") == 0) {
      if (load()) {
        return true;
      } else {
        return false;
      }
    } else if (strcmp(argv[i], "-help") == 0) {
      print_help();
      std::exit(EXIT_SUCCESS);
      break;
    } else if (strcmp(argv[i], "-mcr") == 0) {
      monte_carlo_runs = std::atoi(argv[i + 1]);
      i += 2;
    } else if (strcmp(argv[i], "-mcfsp") == 0) {
      monty_carlo_file_save_path = argv[i + 1];
      i += 2;
    } else {
      Logger::get()->warn("app: flag passed was not valid use -help for help");
      throw std::invalid_argument(argv[i]);
    }
  }

  return true;
}