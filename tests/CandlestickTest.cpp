#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest/doctest.h"
#include "core/candlestick.hpp"

// Basic sanity check: we can assign and read OHLC values
TEST_CASE("CandleStick: simple OHLC assignment") {
    CandleStick c{};  // type from candlestick.hpp

    c.open  = 100;
    c.high  = 110;
    c.low   = 90;
    c.close = 105;

    CHECK(c.open  == 100u);
    CHECK(c.high  == 110u);
    CHECK(c.low   == 90u);
    CHECK(c.close == 105u);
}

// Check simple invariants: high/low bounds relative to open/close
TEST_CASE("CandleStick: high/low bounds are consistent") {
    CandleStick c{};

    c.open  = 100;
    c.high  = 120;
    c.low   = 80;
    c.close = 110;

    CHECK(c.high >= c.open);
    CHECK(c.high >= c.close);
    CHECK(c.low  <= c.open);
    CHECK(c.low  <= c.close);
}

// Check that a default-constructed candle has non-contradictory values
TEST_CASE("CandleStick: default constructed is non-contradictory") {
    CandleStick c{};      // all Price fields default to 0 (unsigned int)

    CHECK(c.high >= c.low);
}
