#pragma once

#include "nlohmann/json.hpp"
#include "core/order.hpp"
#include <chrono>
#include <iostream>
#include <string>

using namespace std::chrono;
using nlohmann::json;

struct CandleStick {
  Price open;
  Price high;
  Price low;
  Price close;
  nanoseconds closing_time;

  uint32_t num_trades;

  void update_price(Price new_price) {
    close = new_price;
    if (new_price > high)
      high = new_price;
    if (new_price < low)
      low = new_price;
    num_trades++;
  }

  CandleStick(Price initial_price = 0)
      : open(initial_price), high(initial_price), low(initial_price),
        close(initial_price), num_trades(0) {};

  void update(Price trade_price, nanoseconds ts) {
    if (num_trades == 0) {
      open = trade_price;
      high = trade_price;
      low = trade_price;
    }

    close = trade_price;

    if (trade_price > high) {
      high = trade_price;
    }
    if (trade_price < low) {
      low = trade_price;
    }

    closing_time = ts;

    num_trades++;
  }

  void reset(Price current_price, nanoseconds ts) {
    open = current_price;
    high = current_price;
    low = current_price;
    close = current_price;

    closing_time = ts;

    num_trades = 0;
  }

  json to_json() const {
    return json{{"open", open},
                {"high", high},
                {"low", low},
                {"close", close},
                {"num_trades", num_trades},
                {"close_time", closing_time.count()}};
  }
};