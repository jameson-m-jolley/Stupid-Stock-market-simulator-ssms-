#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest/doctest.h"
#include "util/config.hpp"
#include <type_traits>

// Just confirm that config is a class type
TEST_CASE("config: is a class type") {
    CHECK(std::is_class<config>::value);
}

// Confirm that config::save and config::load are member functions with signature bool()
TEST_CASE("config: has save() and load() returning bool") {
    using SavePtr = bool (config::*)();
    using LoadPtr = bool (config::*)();

    SavePtr savePtr = &config::save;
    LoadPtr loadPtr = &config::load;

    // If these lines compile, the signature is correct.
    CHECK(savePtr != nullptr);
    CHECK(loadPtr != nullptr);
}
