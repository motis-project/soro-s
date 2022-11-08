#include "doctest/doctest.h"

#include "soro/runtime/eetc/golden_section_search.h"

using namespace soro;
using namespace soro::runtime;

TEST_SUITE_BEGIN("golden_section_search");  // NOLINT

si::time x_squared(si::speed x) { return {(x.val_ * x.val_)}; }
si::time x_squared_moved_right(si::speed x) {
  return {((x.val_ - 5.0F) * (x.val_ - 5.0F))};
};
si::time x_squared_moved_left(si::speed x) {
  return {((x.val_ + 5.0F) * (x.val_ + 5.0F))};
};
si::time x_squared_moved_right_up(si::speed x) {
  return {((x.val_ - 5.0F) * (x.val_ - 5.0F)) + 5.0F};
}
si::time x_squared_moved_right_down(si::speed x) {
  return {((x.val_ - 5.0F) * (x.val_ - 5.0F)) - 5.0F};
}

si::time x_sin(si::speed x) { return {sin(x.val_)}; }

si::time x_squared_neg(si::speed x) { return {-(x.val_ * x.val_)}; }
si::time x_squared_neg_moved_right(si::speed x) {
  return {-((x.val_ - 5.0F) * (x.val_ - 5.0F))};
};
si::time x_squared_neg_moved_left(si::speed x) {
  return {-((x.val_ + 5.0F) * (x.val_ + 5.0F))};
};
si::time x_squared_neg_moved_right_up(si::speed x) {
  return {-((x.val_ - 5.0F) * (x.val_ - 5.0F)) + 5.0F};
}
si::time x_squared_neg_moved_right_down(si::speed x) {
  return {-((x.val_ - 5.0F) * (x.val_ - 5.0F)) - 5.0F};
}

struct optimizer_x_squared {
  static si::time get_at(si::speed x) { return si::time{(x.val_ * x.val_)}; }
};

struct optimizer_x_squared_moved_left {
  static si::time get_at(si::speed x) {
    return si::time{((x.val_ + 5.0F) * (x.val_ + 5.0F))};
  }
};

struct optimizer_x_squared_moved_right_down {
  static si::time get_at(si::speed x) {
    return si::time{((x.val_ - 5.0F) * (x.val_ - 5.0F)) - 5.0F};
  }
};

TEST_CASE("gss-max::parabola::neg") {  // NOLINT
  auto max =
      golden_section_search_max({-10.0F}, {10.0F}, {0.01F}, x_squared_neg);
  CHECK(max == si::speed{0.0F});
}

TEST_CASE("gss-max::parabola-moved-5-right::neg") {  // NOLINT
  auto max = golden_section_search_max({-10.0F}, {10.0F}, {0.01F},
                                       x_squared_neg_moved_right);
  CHECK(max == si::speed{5.0F});
}

TEST_CASE("gss-max::parabola-moved-5-left::neg") {  // NOLINT
  auto max = golden_section_search_max({-10.0F}, {10.0F}, {0.01F},
                                       x_squared_neg_moved_left);
  CHECK(max == si::speed{-5.0F});
}

TEST_CASE("gss-max::parabola-moved-5-right-up::neg") {  // NOLINT
  auto max = golden_section_search_max({-10.0F}, {10.0F}, {0.01F},
                                       x_squared_neg_moved_right_up);
  CHECK(max == si::speed{5.0F});
}

TEST_CASE("gss-max::parabola-moved-5-right-up::neg") {  // NOLINT
  auto max = golden_section_search_max({-10.0F}, {10.0F}, {0.01F},
                                       x_squared_neg_moved_right_down);
  CHECK(max == si::speed{5.0F});
}

TEST_CASE("gss-min::parabola") {  // NOLINT
  auto min = golden_section_search_min({-10.0F}, {10.0F}, {0.01F}, x_squared);
  CHECK(min == si::speed{0.0F});
}

TEST_CASE("gss-min::parabola-moved-5-right") {  // NOLINT
  auto min = golden_section_search_min({-10.0F}, {10.0F}, {0.01F},
                                       x_squared_moved_right);
  CHECK(min == si::speed{5.0F});
}

TEST_CASE("gss-min::parabola-moved-5-left") {  // NOLINT
  auto min = golden_section_search_min({-10.0F}, {10.0F}, {0.01F},
                                       x_squared_moved_left);
  CHECK(min == si::speed{-5.0F});
}

TEST_CASE("gss-min::parabola-moved-5-right-up") {  // NOLINT
  auto min = golden_section_search_min({-10.0F}, {10.0F}, {0.01F},
                                       x_squared_moved_right_up);
  CHECK(min == si::speed{5.0F});
}

TEST_CASE("gss-min::parabola-moved-5-right-up") {  // NOLINT
  auto min = golden_section_search_min({-10.0F}, {10.0F}, {0.01F},
                                       x_squared_moved_right_down);
  CHECK(min == si::speed{5.0F});
}

TEST_CASE("gss-min::sinus::find-min") {  // NOLINT
  auto min = golden_section_search_min({-M_PI}, {M_PI}, {0.01F}, x_sin);
  CHECK(min == si::speed{-M_PI_2});
}

TEST_CASE("gss-max::sinus::find-max") {  // NOLINT
  auto min = golden_section_search_max({-M_PI}, {M_PI}, {0.01F}, x_sin);
  CHECK(min == si::speed{M_PI_2});
}

TEST_CASE("gss::sinus::find-min-and-max") {  // NOLINT
  auto max = golden_section_search({-M_PI}, {M_PI}, {0.01F}, x_sin, false);
  auto min = golden_section_search({-M_PI}, {M_PI}, {0.01F}, x_sin, true);
  CHECK(max == si::speed{M_PI_2});
  CHECK(min == si::speed{-M_PI_2});
}

TEST_CASE("gss_optimizer::parabola::neg") {
  auto optimizer = optimizer_x_squared();

  auto min = golden_section_search_min_optimizer(
      si::speed{-10.0F}, si::speed{10.0F}, si::speed{0.01F}, optimizer);
  CHECK(min == si::speed{0.0F});
}

TEST_CASE("gss_optimizer::parabola-moved-5-left::neg") {
  auto optimizer = optimizer_x_squared_moved_left();

  auto min = golden_section_search_min_optimizer(
      si::speed{-10.0F}, si::speed{10.0F}, si::speed{0.01F}, optimizer);
  CHECK(min == si::speed{-5.0F});
}

TEST_CASE("gss_optimizer::parabola-moved-5-right-up::neg") {
  auto optimizer = optimizer_x_squared_moved_right_down();

  auto min = golden_section_search_min_optimizer(
      si::speed{-10.0F}, si::speed{10.0F}, si::speed{0.01F}, optimizer);
  CHECK(min == si::speed{5.0F});
}

TEST_SUITE_END();  // NOLINT