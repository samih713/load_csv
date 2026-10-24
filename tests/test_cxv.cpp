#include <catch2/catch_test_macros.hpp>
#include <string>

#define CXV_IMPLEMENTATION
#include "../cxv.h"

namespace {

struct pair_t {
  int a;
  int b;
};

void set_pair(char** values, int row, void* ctx) {
  auto* pairs = static_cast<pair_t*>(ctx);
  pairs[row].a = atoi(values[0]);
  pairs[row].b = atoi(values[1]);
}

std::string fixture(const char* name) {
  return FIXTURE_DIR + std::string(name);
}

cxv_cfg_t cfg(char del, bool first_row, size_t rows, size_t n_cols) {
  return {.n_cols = n_cols,
          .r_to_read = rows,
          .del = del,
          .first_row = first_row};
}

}  // namespace

TEST_CASE("cxv_read basic parsing", "[cxv]") {
  pair_t out[5] = {};

  SECTION("reads every row") {
    auto path = fixture("basic.csv");
    REQUIRE(cxv_read(path.c_str(), cfg(',', false, 2, 2), out, set_pair) == 2);
    CHECK(out[0].a == 1);
    CHECK(out[0].b == 2);
    CHECK(out[1].a == 3);
    CHECK(out[1].b == 4);
  }

  SECTION("fewer rows in file than requested") {
    auto path = fixture("basic.csv");
    CHECK(cxv_read(path.c_str(), cfg(',', false, 5, 2), out, set_pair) == 2);
  }

  SECTION("r_to_read limits the rows") {
    auto path = fixture("three_rows.csv");
    CHECK(cxv_read(path.c_str(), cfg(',', false, 2, 2), out, set_pair) == 2);
  }

  SECTION("custom delimiter") {
    auto path = fixture("semicolon.csv");
    REQUIRE(cxv_read(path.c_str(), cfg(';', false, 2, 2), out, set_pair) == 2);
    CHECK(out[1].a == 3);
    CHECK(out[1].b == 4);
  }
}

TEST_CASE("cxv_read header row", "[cxv]") {
  pair_t out[2] = {};
  auto path = fixture("header.csv");

  SECTION("skipped when first_row is set") {
    REQUIRE(cxv_read(path.c_str(), cfg(',', true, 2, 2), out, set_pair) == 2);
    CHECK(out[0].a == 1);
    CHECK(out[0].b == 2);
  }

  SECTION("kept when first_row is unset") {
    REQUIRE(cxv_read(path.c_str(), cfg(',', false, 1, 2), out, set_pair) == 1);
    CHECK(out[0].a == 0);  // atoi("x")
  }
}

TEST_CASE("cxv_read errors", "[cxv]") {
  pair_t out[1] = {};

  SECTION("missing file returns -1") {
    auto status =
        cxv_read("does_not_exist.csv", cfg(',', false, 1, 2), out, set_pair);
    CHECK(status == -1);
  }

  SECTION("no read access returns -1") {
    auto path = fixture("no_read.csv");
    auto status = cxv_read(path.c_str(), cfg(',', false, 1, 2), out, set_pair);
    CHECK(status == -1);
  }

  SECTION("empty file returns 0") {
    auto path = fixture("empty.csv");
    auto status = cxv_read(path.c_str(), cfg(',', false, 1, 2), out, set_pair);
    CHECK(status == 0);
  }
}

TEST_CASE("malformed csv's", "[cxv]") {
  pair_t out[5] = {};

  SECTION("expecting 2 cols middle is 3 cols returns -1") {
    auto path = fixture("middle_uneven_cols.csv");
    auto status = cxv_read(path.c_str(), cfg(',', false, 5, 2), out, set_pair);
    CHECK(status == -1);
  }

  SECTION("expecting 2 cols middle is 1 col returns -1") {
    auto path = fixture("middle_missing_col.csv");
    auto status = cxv_read(path.c_str(), cfg(',', false, 5, 2), out, set_pair);
    CHECK(status == -1);
  }
}

// Known bugs. Hidden by default (they crash under ASan); run with
// `make bugs`.
TEST_CASE("cxv_read last line without trailing newline", "[.][bug]") {
  // the last value of a final line with no '\n' is never emitted
  pair_t out[2] = {};
  auto path = fixture("no_trailing_newline.csv");
  REQUIRE(cxv_read(path.c_str(), cfg(',', false, 2, 2), out, set_pair) == 2);
  CHECK(out[1].a == 3);
  CHECK(out[1].b == 4);
}

TEST_CASE("cxv_read CRLF line endings", "[.][bug]") {
  // '\r' and '\n' each end a value, so values[] overflows by one
  pair_t out[2] = {};
  auto path = fixture("crlf.csv");
  REQUIRE(cxv_read(path.c_str(), cfg(',', false, 2, 2), out, set_pair) == 2);
  CHECK(out[1].b == 4);
}
