#pragma once

#include <vector>

#include "util/config.hpp"
#include "util/logger.hpp"
#include "core/order.hpp"
#include "core/wallet.hpp"

struct Market; // forward declaration

class Trader {
private:
  Wallet wallet;
  std::vector<Quantity> positions;

public:
  Trader(Quantity initial_shares, Price initial_funds, Stock_N num_stocks);

  virtual ~Trader() = default;

  virtual void order_filled(size_t stock_index, Quantity qty, Price price,
                            Side side);

  void set_wallet(Price amount);

  void set_positions(size_t num_stocks, Quantity initial_shares) {
    positions.resize(num_stocks);
    std::fill(positions.begin(), positions.end(), initial_shares);
  }

  void set_position(size_t stock_index, Quantity qty) {
    positions[stock_index] = qty;
  }

  void add_position(size_t stock_index, Quantity qty) {
    positions[stock_index] += qty;
  }

  void sub_position(size_t stock_index, Quantity qty) {
    positions[stock_index] -= qty;
  }

  Quantity get_position(size_t stock_index) const {
    return positions[stock_index];
  }

  bool can_make_trade(Stock_N stock, Price price, Quantity qty, Side side);

  size_t num_positions() const { return positions.size(); }

  Wallet get_wallet() { return wallet; }

  OrderHandle create_order(Chunked_OrderPool &pool, size_t stock_index,
                           Price price, Quantity qty,
                           std::chrono::nanoseconds ts, Side side) {
    return pool.create(static_cast<Stock_N>(stock_index), price, qty, ts, side,
                       this);
  }

  virtual void trade_on_market(Market &market, Chunked_OrderPool &pool,
                               std::chrono::nanoseconds ts);

  json to_json(bool include_positions);
};

class UserTrader : public Trader {
public:
  UserTrader()
      : Trader(0, config::instance().initial_wallet,
               config::instance().num_stocks) {}

  UserTrader(Quantity initial_shares, Price initial_funds, Stock_N num_stocks)
      : Trader(initial_shares, initial_funds, num_stocks) {}

  void order_filled(size_t stock_index, Quantity qty, Price price,
                    Side side) override;
  void trade_on_market(Market &, Chunked_OrderPool &pool,
                       std::chrono::nanoseconds ts) override;
};

class RandomTrader : public Trader {
public:
  RandomTrader()
      : Trader(0, config::instance().initial_wallet,
               config::instance().num_stocks) {}

  RandomTrader(Quantity initial_shares, Price initial_funds, Stock_N num_stocks)
      : Trader(initial_shares, initial_funds, num_stocks) {}
  void trade_on_market(Market &, Chunked_OrderPool &pool,
                       std::chrono::nanoseconds ts) override;
};

inline void create_random_traders(int num_traders, size_t num_stocks,
                                  Price initial_funds, Quantity initial_shares,
                                  std::vector<Trader *> &traders) {
  for (int i = 0; i < num_traders; i++) {
    traders.push_back(
        new RandomTrader(initial_shares, initial_funds, num_stocks));
  }
}