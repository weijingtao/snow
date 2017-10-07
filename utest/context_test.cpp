//
// Created by terrywei on 2017/10/7.
//

#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "context.hpp"

TEST_CASE("contest", "[context]") {
    snow::Context context;
    context.set<int>("a", 10);
    REQUIRE(context.has("a"));
    REQUIRE(context.get<int>("a") == 10);
}
