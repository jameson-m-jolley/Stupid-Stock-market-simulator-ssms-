#include "net/socket.hpp"
#include "util/logger.hpp"
#include "trading/market.hpp"
#include "nlohmann/json.hpp"
#include "sim.hpp"
#include "util/tools.hpp"
#include "core/wallet.hpp"
#include <functional>
#include <string> // for std::string payload
using json = nlohmann::json;
/**
 * @brief Called when a new client connects
 */
void onopen(ws_cli_conn_t client) {
  char *cli = ws_getaddress(client);
  Logger::get()->info("web socket: connection opened, addr: {}", cli);
}

/**
 * @brief Called when a client disconnects
 */
void onclose(ws_cli_conn_t client) {
  char *cli = ws_getaddress(client);
  Logger::get()->info("web socket: connection closed, addr: {}", cli);
  auto &conf = config::instance();
  conf.running = false;
}

/**
 * @brief Called when a message is received
 */
void onmessage(ws_cli_conn_t client, const unsigned char *msg, uint64_t size,
               int type) {
  std::string payload(reinterpret_cast<const char *>(msg),
                      static_cast<size_t>(size));
  Logger::get()->debug(
      "web socket: received message: '{}' ({} bytes), type={}, from={}",
      payload, static_cast<unsigned long long>(size), type,
      ws_getaddress(client));

  ws_sendframe_txt(
      client,
      make_json_string("socket", "\"message was intercepted\"").c_str());
  socket::instance().parse_msg(msg, client);
}

socket::socket() {
  server.host = conf.host;
  server.port = conf.port;
  server.thread_loop = 1;
  server.timeout_ms = 1000;
  server.evs.onopen = &onopen;
  server.evs.onclose = &onclose;
  server.evs.onmessage = &onmessage;
}

void socket::post(std::string msg) {
  ws_sendframe_bcast(server.port, msg.c_str(), msg.size(), 1);
  Logger::get()->debug("web socket: broadcasted message: {}", msg);
}

bool socket::start() {
  Logger::get()->info("web socket: starting ws at ws://{}:{}", server.host,
                      server.port);

  ws_socket(&server);
  return true;
}

void socket::parse_set(json j, ws_cli_conn_t client) {
  try {
    std::string target = j["target"];
    std::string value = j["value"];
    if (target == "mode") {
      ws_sendframe_txt(
          client,
          value.c_str()); // setting mode from web-socket does nothing rn
      conf.mode = std::stoi(value);
    }
    if (target == "size") {
      conf.size = std::stoi(value);
    }
    if (target == "debug") {
      conf.debug = std::stoi(value);
    }
    if (target == "seed") {
      conf.seed = std::stoi(value); // this leads to undeterministic behaviour
    }
    if (target == "update_time") {
      conf.update_time = std::stoi(value);
    }
    if (target == "running") {
      conf.running = std::stoi(value);
    }
    if (target == "paused") {
      conf.paused = std::stoi(value);
    }
    if (target == "port") {
      conf.port = std::stoi(value);
    }
    if (target == "runtime") {
    }
    if (target == "running_forever") {
      conf.running_forever = std::stoi(value);
    }
    if (target == "clamp_config") {
      conf.clamp_config = std::stoi(value);
    }
    if (target == "unsafe") {
      conf.unsafe = std::stoi(value);
    }
    if (target == "should_terminate") {
      conf.should_terminate = std::stoi(value);
    }
    if (target == "load_config") {
      conf.file_path = value.data();
      if (conf.load()) {
        Logger::get()->info(
            "web socket: config loaded from front end: file_name={}",
            conf.file_path);
      } else {
        Logger::get()->error(
            "web socket: could not load from front end: file_name={}",
            conf.file_path);
      }
    }
  } catch (const std::exception &e) {
    Logger::get()->error("web socket: parse_set failed: {}", e.what());
  }
}

json socket::parse_get(json j) {
  json response;
  json r;

  try {
    std::string target = j["target"];

    if (target == "wallet") {
      response["target"] = "wallet";
      response["value"] = 1500;
      r["get"] = response;
      return r;
    }

    if (target == "market") {
      response["target"] = "market";
      response["value"] = "stocks: 10";
      r["get"] = response;
      return r;
    }

    if (target == "config") {
      response["target"] = "config";
      response["value"] = conf.config_to_json();
      r["get"] = response;
      return r;
    }

    if (target == "candles") {
      Stock_N stock = 0;
      int amountOfCandles = 100;

      if (j.contains("value")) {
        if (j["value"].is_object()) {
          auto v = j["value"];
          if (v.contains("stock")) {
            stock = static_cast<Stock_N>(v["stock"].get<int>());
          }
          if (v.contains("amountOfCandles")) {
            amountOfCandles = v["amountOfCandles"].get<int>();
          }
        } else if (j["value"].is_number_integer()) {
          amountOfCandles = j["value"].get<int>();
        }
      }

      auto &sim = sim::instance();
      json candles = sim.get_candles(stock, amountOfCandles);

      if (conf.debug) {
        Logger::get()->info("get candles called {}", j.dump());
        Logger::get()->info("get candles called {}", amountOfCandles);
      }

      response["sim"]["candles"] = {{"stock", stock}, {"candles", candles}};

      return response;
    }

    if (target == "save") {
      if (conf.save()) {
        response["target"] = "save";
        response["value"] = "successful";
        Logger::get()->info("web socket: config saved");
      } else {
        response["target"] = "save";
        response["value"] = "error";
        Logger::get()->error("web socket: config could not save");
      }
      r["get"] = response;
      return r;
    }

    response["target"] = "error";
    response["value"] = "Invalid get target";
    r["get"] = response;
    return r;
  } catch (const std::exception &e) {
    Logger::get()->error("web socket: parse_get failed: {}", e.what());
    response["target"] = "error";
    response["value"] = e.what();
    return response;
  }
}

json socket::parse_user(json j) {
  json response;
  if (!j.contains("stock") || !j.contains("price") || !j.contains("quantity") ||
      !j.contains("side")) {
    response["user"]["tradeStatus"] = "error";
    response["user"]["error"] = "missing fields";
    return response;
  }

  try {
    Stock_N stock = j["stock"];
    Price price = j["price"];
    Quantity qty = j["quantity"];

    std::string side_s = j["side"];
    Side side = (side_s == "buy" || side_s == "Buy") ? Side::Buy : Side::Ask;

    enqueue_user_trade({stock, price, qty, side, conf.runtime});
    response["user"]["tradeStatus"] = "queued";
    return response;

  } catch (const std::exception &e) {
    Logger::get()->error("web socket: parse_user failed: {}", e.what());
    response["user"]["tradeStatus"] = "error";
    response["user"]["error"] = e.what();
    return response;
  }
}

// Handles JSON-based client messages
void socket::parse_msg(const unsigned char *msg, ws_cli_conn_t client) {
  try {
    std::string raw(reinterpret_cast<const char *>(msg));
    json j = json::parse(raw);

    if (!j.contains("command")) {
      ws_sendframe_txt(
          client, make_json_string("error", "\"missing command\"").c_str());
      return;
    }

    std::string command = j["command"];
    json response;

    if (command == "set") {
      if (!j.contains("target")) {
        ws_sendframe_txt(
            client, make_json_string("error", "\"missing target\"").c_str());
        return;
      }

      parse_set(j, client);

      response["server"] = j;
      ws_sendframe_txt(client, response.dump().c_str());
      return;
    }

    if (command == "get") {
      if (!j.contains("target")) {
        ws_sendframe_txt(
            client, make_json_string("error", "\"missing target\"").c_str());
        return;
      }

      response = parse_get(j);

      ws_sendframe_txt(client, response.dump().c_str());
      return;
    }

    if (command == "user") {
      if (!j.contains("trade")) {
        ws_sendframe_txt(
            client, make_json_string("error", "\"missing trade\"").c_str());
        return;
      }

      response = parse_user(j["trade"]);
      ws_sendframe_txt(client, response.dump().c_str());
      return;
    }

  } catch (const std::exception &e) {
    Logger::get()->error("web socket: parse_msg failed: {}", e.what());
  }
}

void socket::enqueue_user_trade(UserTrade trade) {
  user_trade_queue.push(trade);
}

UserTrade socket::dequeue_trade() {
  auto trade = user_trade_queue.front();
  user_trade_queue.pop();
  return trade;
}

bool socket::has_pending_trades() { return !user_trade_queue.empty(); }

void socket::clear_pending_trades() {
  std::queue<UserTrade> empty;
  std::swap(user_trade_queue, empty);
}
