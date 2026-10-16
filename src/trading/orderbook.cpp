#include "trading/orderbook.hpp"
#include "util/config.hpp" // added
#include "util/logger.hpp" // added
#include "trading/trader.hpp"

OrderBook::OrderBook(Price initial_price, Stock_N stock)
    : stock(stock), current_price(initial_price), current_candle() {
  candlesticks.reserve(100);
};

void OrderBook::place_order(OrderHandle o, Chunked_OrderPool &pool,
                            bool should_match) {
  auto &conf = config::instance();
  Price price = pool.price(o);
  Side side = pool.side(o);

  if (side == Side::Buy) {
    buy_orders.push(o);
    if (price > best_bid || best_bid == 0)
      best_bid = price;
  } else {
    sell_orders.push(o);
    if (price < best_ask || best_ask == 0)
      best_ask = price;
  }

  if (conf.debug)
    Logger::get()->info("OrderBook::placed order trader={} price={} side={} "
                        "best bid={} best ask={}",
                        static_cast<void *>(pool.owner(o)), price,
                        static_cast<unsigned long long>(side), best_bid,
                        best_ask);

  if (should_match)
    match_orders(pool);
}

void OrderBook::match_orders(Chunked_OrderPool &pool) {
  auto &conf = config::instance();

  if (conf.debug)
    Logger::get()->info("OrderBook::match_orders called current price={}",
                        current_price);

  if (best_bid == 0 || best_ask == 0) {
    return;
  }

  if (best_bid < best_ask) {
    return;
  }

  while (!buy_orders.empty() && !sell_orders.empty() && best_bid >= best_ask) {
    OrderHandle buy = buy_orders.top();
    OrderHandle sell = sell_orders.top();

    Quantity buy_qty = pool.quantity(buy);
    Quantity sell_qty = pool.quantity(sell);
    Trader *buy_owner = pool.owner(buy);
    Trader *sell_owner = pool.owner(sell);

    if (conf.debug && best_bid < best_ask) {
      Logger::get()->info("Desync detected: best_bid={} < best_ask={}",
                          best_bid, best_ask);
    }

    // Compute trade qty & price
    Quantity trade_qty = std::min(buy_qty, sell_qty);
    Price trade_price;

    if (pool.timestamp(buy) > pool.timestamp(sell)) {
      trade_price = best_ask;
    } else {
      trade_price = best_bid;
    }

    // Update quantity
    buy_qty -= trade_qty;
    sell_qty -= trade_qty;

    // cursed un absracted set since order pool interface doesn't have setters
    buy.chunk->quantities[buy.index] = buy_qty;
    sell.chunk->quantities[sell.index] = sell_qty;

    if (buy_qty == 0)
      buy_orders.pop();
    if (sell_qty == 0)
      sell_orders.pop();

    buy_owner->order_filled(stock, trade_qty, trade_price, Side::Buy);
    sell_owner->order_filled(stock, trade_qty, trade_price, Side::Ask);

    auto now = steady_clock::now();
    auto current_time = now - conf.start_time;

    current_candle.update(trade_price, current_time);

    // Update current price to reflect the last trade (L1 ask of orderbook)
    current_price = trade_price;
    trade_volume++;

    // Update the best bid & ask to reflect the new L1 orders
    update_best_bid_ask(pool);

    if (conf.debug) {
      Logger::get()->info("OrderBook::match_loop trade (stock={} price={} "
                          "qty={} sellRem={} buyRem={})",
                          stock, trade_price, trade_qty, sell_qty, buy_qty);
      Logger::get()->info(
          "OrderBook::match_loop snapshot (price={} best_bid={} best_ask={})",
          current_price, best_bid, best_ask);
    }
  }
}

void OrderBook::update_candlesticks(nanoseconds ts) {
  if (candlesticks.size() == candlesticks.max_size()) {
    candlesticks.erase(candlesticks.begin());
  }

  candlesticks.push_back(current_candle);
  current_candle.reset(current_price, ts);
}

json OrderBook::get_json_candles(int amountOfCandles) {
  json j = json::array();

  size_t amount = static_cast<size_t>(amountOfCandles);

  if (amount > candlesticks.size()) {
    amount = candlesticks.size();
  }

  for (int i = candlesticks.size() - amount; amount > 0; i++) {
    j.push_back(candlesticks[i].to_json());
    amount--;
  }

  return j;
}