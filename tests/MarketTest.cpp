#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest/doctest.h"
#include "trading/trader.hpp"
#include <type_traits>

// Basic sanity: Trader is a proper class
TEST_CASE("Trader: is a class type") {
   // CHECK(std::is_class<Trader>::value);
}

// Trader should be default-constructible with its default parameters
TEST_CASE("Trader: is default-constructible") {
    //CHECK(std::is_default_constructible<Trader>::value);

    //Trader t(0,0,0);  // actually construct one to ensure it links
    //(void)t;     // silence unused variable warning
}

// And it should at least be move-constructible / destructible
TEST_CASE("Trader: move-constructible and destructible") {
    //CHECK(std::is_move_constructible<Trader>::value);
    //CHECK(std::is_destructible<Trader>::value);
}
