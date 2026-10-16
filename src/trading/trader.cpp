#include "trading/trader.hpp"
#include "trading/market.hpp"
#include "net/socket.hpp"

Trader::Trader(Quantity initial_shares, Price initial_funds,
               Stock_N num_stocks) {
  wallet = {static_cast<int>(initial_funds), 0};
  set_positions(num_stocks, initial_shares);
}

void Trader::order_filled(size_t stock_index, Quantity qty, Price price,
                          Side side) {
  auto &conf = config::instance();

  if (side == Side::Ask) {
    wallet.Deposit(0, price * qty);
    sub_position(stock_index, qty); // < underflowable
  } else {
    wallet.Withdraw(0, price * qty);
    add_position(stock_index, qty);
  }

  if (conf.debug) {
    Logger::get()->info(
        "sim:[trader:{}] order_filled called stock={} side={}, posAft={}",
        static_cast<void *>(this), stock_index,
        static_cast<unsigned long long>(side), get_position(stock_index));
  }
}

bool Trader::can_make_trade(Stock_N stock, Price price, Quantity qty,
                            Side side) {
  auto wallet_x = wallet.Getwallet();
  if (side == Side::Ask && get_position(stock) == 0) {
    return false;
  }

  if (side == Side::Buy &&
      static_cast<unsigned long long>(
          wallet.convertToCents(wallet_x.first, wallet_x.second)) <
          static_cast<unsigned long long>(price * qty)) {
    return false;
  }

  return true;
}

void Trader::trade_on_market(Market &, Chunked_OrderPool &,
                             std::chrono::nanoseconds) {}

json Trader::to_json(bool include_positions) {
  json j;
  j["wallet"] = wallet.Getwallet();

  if (include_positions) {
    json positions = json::array();
    for (size_t i = 0; i < num_positions(); i++) {
      positions.push_back(get_position(i));
    }
    j["positions"] = positions;
  }

  return j;
}

void UserTrader::order_filled(size_t stock_index, Quantity qty, Price price,
                              Side side) {
  Trader::order_filled(stock_index, qty, price, side);

  auto &server = socket::instance();
  Wallet wallet = get_wallet();
  auto wallet_x = wallet.Getwallet();

  json response;
  response["user"]["tradeStatus"] = "filled";
  response["user"]["trade"] = {
      {"stock", stock_index},
      {"quantity", qty},
      {"side", side == Side::Buy ? "buy" : "sell"},
      {"wallet", wallet.convertToCents(wallet_x.first, wallet_x.second)}};
  server.post(response.dump().c_str());
}

void UserTrader::trade_on_market(Market &market, Chunked_OrderPool &pool,
                                 std::chrono::nanoseconds) {
  socket &server = socket::instance();
  json response;

  while (server.has_pending_trades()) {
    auto trade = server.dequeue_trade();

    if (!can_make_trade(trade.stock, trade.price, trade.quantity, trade.side)) {
      continue;
    }

    response["user"]["tradeStatus"] = "on_market";

    auto handle = create_order(pool, trade.stock, trade.price, trade.quantity,
                               trade.timestamp, trade.side);

    server.post(response.dump().c_str());
    market.place_order(pool, trade.stock, handle);
    Logger::get()->info("sim:[strategy] user trade executed: stock={}, "
                        "price={}, qty={}",
                        trade.stock, trade.price, trade.quantity);
  }
}

void RandomTrader::trade_on_market(Market &market, Chunked_OrderPool &pool,
                                   std::chrono::nanoseconds ts) {
  auto &conf = config::instance();
  static std::mt19937 rng(conf.seed);

  std::uniform_int_distribution<int> variation_dist(5, 50);
  std::uniform_int_distribution<int> side_dist(0, 1);
  std::uniform_int_distribution<int> qty_dist(1, 20);
  std::uniform_int_distribution<int> aggressive_dist(0, 100);

  Stock_N number_of_stocks = market.get_num_stocks();
  std::uniform_int_distribution<Stock_N> stock_dist(0, number_of_stocks - 1);
  std::uniform_int_distribution<int> num_trades_dist(1, 10);

  std::vector<Stock_N> updated_stocks;

  int num_trades =
      std::min(num_trades_dist(rng), static_cast<int>(number_of_stocks));

  for (int i = 0; i < num_trades; i++) {
    Stock_N stock = stock_dist(rng);
    Quantity qty = qty_dist(rng);
    Side side = (side_dist(rng) == 0) ? Side::Buy : Side::Ask;

    Price current_price = market.get_price(stock);

    Price variation = (current_price * variation_dist(rng)) / 1000;
    Price price;
    bool is_aggressive = (aggressive_dist(rng) < 70);
    if (is_aggressive) {
      price = (side == Side::Buy) ? current_price + variation
                                  : current_price - variation;
    } else {
      price = (side == Side::Buy) ? current_price - variation
                                  : current_price + variation;
    }

    if (!can_make_trade(stock, price, qty, side)) {
      continue;
    }

    auto handle = create_order(pool, stock, price, qty, ts, side);
    market.place_order(pool, stock, handle);
  }
}
