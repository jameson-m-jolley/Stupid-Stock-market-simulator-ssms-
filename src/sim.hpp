#pragma once

#include "trading/market.hpp"
#include "net/socket.hpp"

using namespace std::chrono;

class sim {
private:
  sim() {};

  sim(const sim &) = delete;
  sim &operator=(const sim &) = delete;
  sim(sim &&) = delete;
  sim &operator=(sim &&) = delete;

  config &conf = config::instance();
  socket &server = socket::instance();

  Chunked_OrderPool pool;
  Market market;
  std::vector<Trader *> traders;
  UserTrader user;

  void initialize_sim() {

    market.initialize_stocks(conf.num_stocks, conf.stock_price);
    market.init_mock_trader(traders);

    create_random_traders(conf.num_traders, conf.num_stocks,
                          conf.initial_wallet, 10, traders);

    Trader *mock_trader = traders[0];
    market.seed_market(conf.seed_orders, nanoseconds(0), pool, mock_trader);

    user = UserTrader(10, conf.initial_wallet, conf.num_stocks);
    traders.push_back(&user);

    conf.start_time = steady_clock::now();
    send_start();

    Logger::get()->info("sim: initialization done");
  }

  void deinitialize_sim() {
    Logger::get()->info("sim: starting deinitialization");

    market.number_of_stocks = 0;
    conf.running = false;
    conf.paused = false;

    market.orderbooks.clear();
    pool.reset();

    for (auto t : traders) {
      if (t != &user) {
        delete t;
      }
    }

    traders.clear();
    server.clear_pending_trades();

    Logger::get()->info("sim: deinitialization complete");
  }

public:
  bool run_sim() {
    initialize_sim();

    auto elapsed_time = nanoseconds(0);
    auto total_correction = nanoseconds(0);

    while (conf.running && !conf.should_terminate) {
      if (conf.paused) {
        Logger::get()->info("sim: paused");
        send_update(elapsed_time, "paused");
        sleep(1);
        continue;
      }

      auto iteration_start = steady_clock::now();

      for (auto &trader : traders) {
        trader->trade_on_market(market, pool, elapsed_time);
      }

      market.update(elapsed_time);

      auto iteration_end = steady_clock::now();
      auto iteration_duration = iteration_end - iteration_start;

      total_correction += iteration_duration;

      auto wall_time = iteration_end - conf.start_time;
      elapsed_time = wall_time - total_correction;

      send_update(elapsed_time, "okay");

      auto iteration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                              iteration_duration)
                              .count();
      Logger::get()->info("sim: trader strategies executed in {} ms",
                          iteration_ms);

      if (!conf.running_forever) {
        conf.running = (conf.runtime >= elapsed_time);
      }

      auto target_interval = milliseconds(static_cast<int>(conf.update_time));
      auto sleep_time =
          target_interval - duration_cast<milliseconds>(iteration_duration);
      if (sleep_time.count() > 0) {
        std::this_thread::sleep_for(sleep_time);
      }
    }
    deinitialize_sim();

    return true;
  }

  void send_start() {
    json j;
    j["sim"]["start"] = {{"num_stocks", conf.num_stocks},
                         {"init_wallet", conf.initial_wallet},
                         {"time_passed", 0}};

    Logger::get()->info("sim: init = {}", j.dump());
    server.post(j.dump());
  }

  void send_update(std::chrono::nanoseconds elapsed_time,
                   const std::string &status) {
    json prices = get_current_prices();
    json volumes = get_current_volumes();

    json stock_data;

    stock_data = {{"prices", prices}, {"volumes", volumes}};

    json j;
    j["sim"]["update"] = {{"time_passed", elapsed_time.count()},
                          {"status", status},
                          {"stock_data", stock_data}};

    server.post(j.dump());
  }

  json get_candles(Stock_N stock, int amountOfCandles) {
    if (!conf.running) {
      return json::array();
    }

    return market.get_json_candles(stock, amountOfCandles);
  }

  json get_current_prices() {
    if (!conf.running) {
      return json::array();
    }

    json j = json::array();

    for (size_t i = 0; i < conf.num_stocks; i++) {
      j.push_back(market.get_price(i));
    }

    return j;
  }

  json get_current_volumes() {
    if (!conf.running) {
      return json::array();
    }

    json j = json::array();

    for (size_t i = 0; i < conf.num_stocks; i++) {
      j.push_back(market.get_volume(i));
    }

    return j;
  }

  static sim &instance() {
    static sim sim;
    return sim;
  }
};
