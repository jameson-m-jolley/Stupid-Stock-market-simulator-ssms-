#pragma once

#include "util/config.hpp"
#include "nlohmann/json.hpp"
#include "core/order.hpp"
#include "util/tools.hpp"
#include "ws.h"
#include <cstring>
#include <iostream>
#include <pthread.h>
#include <queue>
#include <stdio.h>
#include <string>
#include <strings.h>
using json = nlohmann::json;

struct UserTrade {
  Stock_N stock;
  Price price;
  Quantity quantity;
  Side side;
  std::chrono::nanoseconds timestamp;
};

class socket {
private:
  socket();

  socket(const socket &) = delete;
  socket &operator=(const socket &) = delete;
  socket(socket &&) = delete;
  socket &operator=(socket &&) = delete;

  struct ws_server server;
  config &conf = config::instance();

  void parse_set(json j, ws_cli_conn_t client);
  json parse_get(json j);
  json parse_user(json j);

  std::queue<UserTrade> user_trade_queue;

public:
  static socket &instance() {
    static socket socket;
    return socket;
  }
  // Declarations only — the actual logic is in socket.cpp
  void parse_msg(const unsigned char *msg, ws_cli_conn_t client);
  std::string get() { return "hello"; }
  void post(std::string msg);
  socket(config *config);
  bool start();

  void enqueue_user_trade(UserTrade trade);
  UserTrade dequeue_trade();
  bool has_pending_trades();
  void clear_pending_trades();
};