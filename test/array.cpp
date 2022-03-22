//
// Created by Roel Hemerik on 22/03/2022.
//

#include "bulk_test_common.hpp"
#include "set_backend.hpp"

extern environment env;

void test_array() {
  env.spawn(env.available_processors(), [](auto& world) {

    BULK_SECTION("Array Move Constructor") {

      auto arr = bulk::array<int>(world,20);

      BULK_CHECK(arr.size() == 20, "Array size incorrectly initialised.");

      auto moved_arr = bulk::array<int>(std::move(arr));

      BULK_CHECK(moved_arr.size() == 20, "Array size incorrectly moved");
    }
  });
}