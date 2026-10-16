#include "wallet.hpp"
#include <cstddef>
#include <cstdio>
#include <climits> // For INT_MAX
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest/doctest.h"
#include "nlohmann/json.hpp"
#include "util/logger.hpp"
#include "util/config.hpp"
#include "sim.hpp"

TEST_CASE("init"){
    char arg0[] = "sim"; 
    char *args[] = {arg0,NULL};
    init_config_and_logger(1, args);
    Wallet tester(1,1);
    auto [a,b] = tester.Getwallet(); 
    CHECK(a == 1);
    CHECK(b == 1);
    Wallet tester1(1,101);
    auto [c,d] = tester1.Getwallet();
    CHECK(c == 2);
    CHECK(d == 1);
}

TEST_CASE("convertToCents"){
    Wallet tester3(0,0);
    auto cent = tester3.convertToCents(0,99);
    CHECK(cent == 99);
    cent = tester3.convertToCents(1,99);
    CHECK(cent == 199);
    cent = tester3.convertToCents(0,0);
    CHECK(cent == 0);
    cent = tester3.convertToCents(1,0);
    CHECK(cent == 100);
    //negative 
    cent = tester3.convertToCents(-1,0);
    CHECK(cent == -100);
    cent = tester3.convertToCents(0,-1);
    CHECK(cent == -1);
    cent = tester3.convertToCents(1,-1);
    CHECK(cent == 99);
}
TEST_CASE("convertToDollars"){
    Wallet testerCTD(0,0);
    auto [a,b]= testerCTD.convertToDollars(100);
    CHECK( a == 1);
    CHECK( b == 0);
    auto [c,d] = testerCTD.convertToDollars(-100);
    CHECK( c == -1);
    CHECK( d == 0);
}
TEST_CASE("add"){
    Wallet testeradd(0,0);
    // added basic
    auto [a,b] = testeradd.add(1,1);
    CHECK( a == 1 );
    CHECK( b == 1 );
    // adding 0's
    auto [c,d] = testeradd.add(0,0);
    CHECK( c == 0 );
    CHECK( d == 0 );
    //adding a neg balance
    auto [e,f] = testeradd.add(-1,-1);
    CHECK( e == -1 );
    CHECK( f == -1 );
    // adding large cents
    auto [h,i] = testeradd.add(0,INT32_MAX);
    CHECK( h == INT32_MAX/100 );
    CHECK( i == INT32_MAX%100  );
}
TEST_CASE("sub"){
    Wallet testersub(0,0);
    auto [a,b] = testersub.sub(0,0);
    CHECK( a == 0 );
    CHECK( b == 0 );
    //large numbers
    auto [c,d] = testersub.sub(0,INT32_MAX);
    CHECK( c == (INT32_MIN+1)/100 );
    CHECK( d == (INT32_MIN+1)%100 );
    //subtracting negative numbers 
    auto [e,f] = testersub.sub(-1,-1);
    CHECK( e == 1 );
    CHECK( f == 1 );

    auto [h,i] = testersub.sub(0,100);
    CHECK( h == -1 );
    CHECK( i == 0 );
}
TEST_CASE("Withdraw"){
    Wallet testerWithdaw(1000,99);
    testerWithdaw.Withdraw(0,99);
    auto [a,b] = testerWithdaw.Getwallet();
    CHECK(a == 1000);
    CHECK(b == 0);
    testerWithdaw.Withdraw(-1,-1);
    auto [c,d] = testerWithdaw.Getwallet();
    CHECK(c == 1000);
    CHECK(d == 0);// can't withdraw negative money
    //CHECK( false );
}
TEST_CASE("tostring"){
    //this fails if the string is not valid json 
    Wallet testertostring(0,0);
    std::string str = testertostring.toString();
    CHECK( nlohmann::json::accept(str) == true );
}
