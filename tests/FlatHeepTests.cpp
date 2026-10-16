#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest/doctest.h"
#include "core/FlatHeap.hpp"
#include "core/order.hpp"
#include "trading/trader.hpp"

TEST_CASE("empty?"){
    FlatHeap<OrderHandle, OrderComparator> heep; //make the heep
    CHECK(heep.empty());
}

TEST_CASE("size"){
    FlatHeap<OrderHandle, OrderComparator> heep; //make the heep
    CHECK(heep.size() == 0);
    OrderHandle g,h,i;
    heep.push(g);
    heep.push(h);
    heep.push(i);
    CHECK(heep.size() == 3);
}

TEST_CASE("top"){
    FlatHeap<OrderHandle, OrderComparator> heep; //make the heep
    OrderHandle g,h,i;
    heep.push(g);
    heep.push(h);
    heep.push(i);
    CHECK(heep.top().index == g.index);
    heep.pop();
    CHECK(heep.top().index == i.index);
 
}


TEST_CASE("clear"){
    FlatHeap<OrderHandle, OrderComparator> heep; //make the heep
    CHECK(heep.size() == 0);
    OrderHandle g,h,i;
    heep.push(g);
    heep.push(h);
    heep.push(i);
    heep.clear();
    CHECK(heep.size() == 0);
}