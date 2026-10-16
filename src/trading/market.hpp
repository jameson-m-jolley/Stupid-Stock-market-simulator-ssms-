#pragma once

#include "util/logger.hpp"
#include "trading/orderbook.hpp"
#include <iostream>
#include <random>
#include <string>

/* Market

The market structure wraps the orderbooks.

init stocks intializes the orderbooks in place

open_market creates a mock trader so traders have something to match

match_all_orders iterates over all the orderbooks and matches the orders they
have stored
*/

struct Market {
  std::vector<OrderBook> orderbooks;
  Stock_N number_of_stocks;

  void initialize_stocks(Stock_N num_stocks, Price init_price) {
    number_of_stocks = num_stocks;
    for (Stock_N stock = 0; stock < num_stocks; stock++) {
      orderbooks.emplace_back(init_price, stock);
    }
  }

  inline void init_mock_trader(std::vector<Trader *> &traders) {
    traders.emplace_back(new RandomTrader(0, 0, number_of_stocks));
  }

  void place_order(Chunked_OrderPool &pool, Stock_N stock, OrderHandle o) {
    orderbooks[stock].place_order(o, pool);
  }

  void seed_market(uint8_t num_seed_orders, std::chrono::nanoseconds ts,
                   Chunked_OrderPool &pool, Trader *mock_trader) {
    thread_local std::mt19937 rng(std::random_device{}());

    std::uniform_int_distribution<int> variation_dist(5, 50);
    std::uniform_int_distribution<int> qty_dist(10, 50);

    Logger::get()->info("sim:[market] seed market called mock trader={}",
                        static_cast<void *>(mock_trader));

    for (Stock_N stock = 0; stock < number_of_stocks; stock++) {
      Price initial_price = orderbooks[stock].get_price();

      for (uint8_t i = 0; i < num_seed_orders; i++) {
        Quantity qty = qty_dist(rng);
        Price variation = (initial_price * variation_dist(rng)) / 1000;
        Price price = initial_price - variation;

        OrderHandle order =
            pool.create(stock, price, qty, ts, Side::Buy, mock_trader);
        orderbooks[stock].place_order(order, pool, false);
      }

      for (uint8_t i = 0; i < num_seed_orders; i++) {
        Quantity qty = qty_dist(rng);
        Price variation = (initial_price * variation_dist(rng)) / 1000;
        Price price = initial_price + variation;

        mock_trader->add_position(stock, qty);

        OrderHandle order =
            pool.create(stock, price, qty, ts, Side::Ask, mock_trader);
        orderbooks[stock].place_order(order, pool, false);
      }
    }
  }

  void update(nanoseconds ts) {
    for (Stock_N stock = 0; stock < number_of_stocks; stock++) {
      orderbooks[stock].update_candlesticks(ts);
    }
  }

  Stock_N get_num_stocks() { return number_of_stocks; }
  Price get_price(Stock_N stock) { return orderbooks[stock].get_price(); }
  int get_volume(Stock_N stock) { return orderbooks[stock].get_volume(); }

  json get_json_candles(Stock_N stock, int amountOfCandles) {
    return orderbooks[stock].get_json_candles(amountOfCandles);
  }
};