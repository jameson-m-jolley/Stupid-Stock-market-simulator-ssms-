#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest/doctest.h"
#include "nlohmann/json.hpp"
#include "trading/orderbook.hpp"
#include "util/config.hpp"
#include "util/logger.hpp"
#include "sim.hpp"
#include "core/candlestick.hpp"
#include "trading/trader.hpp"


TEST_CASE("place_order"){
    char arg0[] = "sim"; 
    char *args[] = {arg0,NULL};
    init_config_and_logger(1, args); // this is a singleton so only needs to be called once
    //this makes a basic orderbook
    OrderBook testbook(100, 1);
    Chunked_OrderPool pool;
    struct OrderHandle order = pool.create(1, 105, 10, std::chrono::nanoseconds(1000), Side::Buy, nullptr);

    testbook.place_order(order, pool);
    CHECK(testbook.get_price() == 100); // price should not change on place order
}

TEST_CASE("update_best_bid_ask"){
    //this makes a basic orderbook
    OrderBook testbook(100, 1);
    Chunked_OrderPool pool;
    struct OrderHandle order = pool.create(1, 105, 10, std::chrono::nanoseconds(1000), Side::Buy, nullptr);
    testbook.place_order(order, pool);
    order = pool.create(1, 120, 10, std::chrono::nanoseconds(1000), Side::Buy, nullptr);
    testbook.place_order(order, pool);
    testbook.update_best_bid_ask(pool);
    CHECK(testbook.get_price() == 120); // price should change on place order
}

TEST_CASE("match_orders"){
    OrderBook testbook(100, 1);
    Chunked_OrderPool pool;
    struct OrderHandle order = pool.create(1, 105, 10, std::chrono::nanoseconds(1000), Side::Buy, nullptr);
    testbook.place_order(order, pool);

}
