#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest/doctest.h"
#include "util/tools.hpp"
#include "nlohmann/json.hpp"

TEST_CASE("make_json_string"){
    auto json_str = make_json_string("key", "value");
    CHECK(nlohmann::json::accept(json_str) == true);
}
