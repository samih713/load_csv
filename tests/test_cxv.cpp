#include <catch2/catch_test_macros.hpp>
#include <string>

#define CXV_IMPLEMENTATION
#include "../cxv.h"

namespace {

struct pair_t {
  int a;
  int b;
};

void set_pair(char** values, void* p) {
  auto* pair = static_cast<pair_t*>(p);
  pair->a = atoi(values[0]);
  pair->b = atoi(values[1]);
}

std::string fixture(const char* name) { return FIXTURE_DIR + std::string(name); }

cxv_cfg_t cfg(char del, bool first_row, size_t rows) {
  return {.n_cols = 2,
          .e_size = sizeof(pair_t),
          .r_to_read = rows,
          .del = del,
          .first_row = first_row};
}

}  // namespace

TEST_CASE("cxv_read basic parsing", "[cxv]") {
  pair_t out[5] = {};

  SECTION("reads every row") {
    auto path = fixture("basic.csv");
    REQUIRE(cxv_read(path.c_str(), cfg(',', false, 2), out, set_pair) == 2);
    CHECK(out[0].a == 1);
    CHECK(out[0].b == 2);
    CHECK(out[1].a == 3);
    CHECK(out[1].b == 4);
  }

  SECTION("fewer rows in file than requested") {
    auto path = fixture("basic.csv");
    CHECK(cxv_read(path.c_str(), cfg(',', false, 5), out, set_pair) == 2);
  }

  SECTION("r_to_read limits the rows") {
    auto path = fixture("three_rows.csv");
    CHECK(cxv_read(path.c_str(), cfg(',', false, 2), out, set_pair) == 2);
  }

  SECTION("custom delimiter") {
    auto path = fixture("semicolon.csv");
    REQUIRE(cxv_read(path.c_str(), cfg(';', false, 2), out, set_pair) == 2);
    CHECK(out[1].a == 3);
    CHECK(out[1].b == 4);
  }
}

TEST_CASE("cxv_read header row", "[cxv]") {
  pair_t out[2] = {};
  auto path = fixture("header.csv");

  SECTION("skipped when first_row is set") {
    REQUIRE(cxv_read(path.c_str(), cfg(',', true, 2), out, set_pair) == 2);
    CHECK(out[0].a == 1);
    CHECK(out[0].b == 2);
  }

  SECTION("kept when first_row is unset") {
    REQUIRE(cxv_read(path.c_str(), cfg(',', false, 1), out, set_pair) == 1);
    CHECK(out[0].a == 0);  // atoi("x")
  }
}

TEST_CASE("cxv_read errors", "[cxv]") {
  pair_t out[1] = {};

  SECTION("missing file returns 0") {
    CHECK(cxv_read("does_not_exist.csv", cfg(',', false, 1), out, set_pair) == 0);
  }

  SECTION("empty file returns 0") {
    auto path = fixture("empty.csv");
    CHECK(cxv_read(path.c_str(), cfg(',', false, 1), out, set_pair) == 0);
  }
}

// Known bugs. Hidden by default (they crash under ASan); run with
// `make bugs`.
TEST_CASE("cxv_read last line without trailing newline", "[.][bug]") {
  // the last value of a final line with no '\n' is never emitted
  pair_t out[2] = {};
  auto path = fixture("no_trailing_newline.csv");
  REQUIRE(cxv_read(path.c_str(), cfg(',', false, 2), out, set_pair) == 2);
  CHECK(out[1].a == 3);
  CHECK(out[1].b == 4);
}

TEST_CASE("cxv_read CRLF line endings", "[.][bug]") {
  // '\r' and '\n' each end a value, so values[] overflows by one
  pair_t out[2] = {};
  auto path = fixture("crlf.csv");
  REQUIRE(cxv_read(path.c_str(), cfg(',', false, 2), out, set_pair) == 2);
  CHECK(out[1].b == 4);
}
