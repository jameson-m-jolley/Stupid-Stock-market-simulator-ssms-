#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest/doctest.h"
#include "util/logger.hpp"

// Make sure Logger::get() returns a valid spdlog logger
TEST_CASE("Logger::get returns non-null logger") {
    auto &loggerPtr = Logger::get();   // std::shared_ptr<spdlog::logger>&
    CHECK(bool(loggerPtr));           // shared_ptr is not null
}

// Basic smoke test: logging functions do not throw
TEST_CASE("Logger basic logging calls do not throw") {
    auto &loggerPtr = Logger::get();
    REQUIRE(bool(loggerPtr));

    CHECK_NOTHROW(loggerPtr->info("info message from LoggerTest"));
    CHECK_NOTHROW(loggerPtr->warn("warn message from LoggerTest"));
    CHECK_NOTHROW(loggerPtr->error("error message from LoggerTest"));

    // Optional: flush to make sure the sink handles it
    CHECK_NOTHROW(loggerPtr->flush());
}
