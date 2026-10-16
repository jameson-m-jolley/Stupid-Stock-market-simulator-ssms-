#pragma once

#include "core/FlatHeap.hpp"
#include <queue>

#include "core/candlestick.hpp"
#include "trading/trader.hpp"

/*
The orderbook stores sell & buy orders in level structure with a priority queue,
Upon every placed order it calls the matchcing engine to assess if it fulfills
any orders before making a callback to the trader.
*/
class OrderBook {
private:
  FlatHeap<OrderHandle, OrderComparator> buy_orders;
  FlatHeap<OrderHandle, OrderComparator> sell_orders;

  Stock_N stock;
  Price current_price;
  int trade_volume = 0;

  CandleStick current_candle;
  std::vector<CandleStick> candlesticks;

  Price best_bid;
  Price best_ask;

public:
  explicit OrderBook(Price initial_price, Stock_N stock);

  void place_order(OrderHandle o, Chunked_OrderPool &pool,
                   bool should_match = true);
  void match_orders(Chunked_OrderPool &pool);

  inline void update_best_bid_ask(Chunked_OrderPool &pool) {
    best_bid = buy_orders.empty() ? 0 : pool.price(buy_orders.top());
    best_ask = sell_orders.empty() ? 0 : pool.price(sell_orders.top());
  };

  void update_candlesticks(nanoseconds ts);

  // getter & setters
  Price get_price() const { return current_price; };
  int get_volume() const { return trade_volume; };
  void set_price(Price price) { current_price = price; };

  std::string get_candle() const { return current_candle.to_json(); }
  std::vector<CandleStick> get_candles() const { return candlesticks; };

  json get_json_candles(int amountOfCandles);
};