#include "soro/infrastructure/graph/construction/get_dir.h"

#include <string>

#include "utl/verify.h"

#include "soro/base/soro_types.h"

#include "soro/utls/sassert.h"
#include "soro/utls/string.h"

#include "soro/infrastructure/graph/detail/element_array_idx.h"
#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/section.h"
#include "soro/infrastructure/parsers/iss/iss_string_literals.h"

namespace soro::infra {

using namespace utls;

simple_switch::direction get_dir(simple_switch const&,
                                 std::string const& node_name) {
  using enum simple_switch::direction;

  switch (str_hash(node_name)) {
    case str_hash(SWITCH_START): return start;
    case str_hash(SWITCH_STEM): return stem;
    case str_hash(SWITCH_BRANCH_LEFT):
    case str_hash(SWITCH_BRANCH_RIGHT): return branch;
    default:
      throw utl::fail("could not determine switch direction from node name {}",
                      node_name);
  }
}

detail::element_array_idx get_dir(std::string const& node_name,
                                  section::position const pos) {

  switch (str_hash(node_name)) {
    case str_hash(KM_JUMP_START):
    case str_hash(LINE_SWITCH_ZERO):
    case str_hash(SWITCH_START):
    case str_hash(CROSS_START_LEFT):
    case str_hash(CROSS_SWITCH_START_LEFT): return 0;
    case str_hash(KM_JUMP_END):
    case str_hash(LINE_SWITCH_ONE):
    case str_hash(SWITCH_STEM):
    case str_hash(CROSS_END_LEFT):
    case str_hash(CROSS_SWITCH_END_LEFT): return 1;
    case str_hash(SWITCH_BRANCH_LEFT):
    case str_hash(SWITCH_BRANCH_RIGHT):
    case str_hash(CROSS_START_RIGHT):
    case str_hash(CROSS_SWITCH_START_RIGHT): return 2;
    case str_hash(CROSS_END_RIGHT):
    case str_hash(CROSS_SWITCH_END_RIGHT): return 3;
    case str_hash(BORDER): return is_start(pos) ? 1 : 0;

    default: return 0;
  }
}

template <typename Direction>
bool in_range(soro::size_t const i) {
  return i <= static_cast<soro::size_t>(Direction::invalid);
}

cross::direction get_dir(cross const&, std::string const& node_name) {
  auto const int_dir = get_dir(node_name, section::position::invalid);

  utls::sassert(in_range<cross::direction>(int_dir),
                "int direction {} not in range <= {}", int_dir, 3);

  return static_cast<cross::direction>(int_dir);
}

soro::size_t get_dir(track_element const&, std::string const&) { return 0; }
soro::size_t get_dir(end_element const&, std::string const&) { return 0; }
soro::size_t get_dir(simple_element const&, std::string const&) { return 0; }

simple_switch::direction get_switch_dir(std::string const& node_name) {
  using enum simple_switch::direction;

  switch (str_hash(node_name)) {
    case str_hash(SWITCH_START): return start;
    case str_hash(SWITCH_STEM): return stem;
    case str_hash(SWITCH_BRANCH_LEFT):
    case str_hash(SWITCH_BRANCH_RIGHT): return branch;
    default:
      throw utl::fail("could not determine switch direction from node name {}",
                      node_name);
  }
}

cross::direction get_cross_dir(std::string const& node_name) {
  using enum cross::direction;

  switch (str_hash(node_name)) {
    case str_hash(CROSS_START_LEFT):
    case str_hash(CROSS_SWITCH_START_LEFT): return start_left;
    case str_hash(CROSS_END_LEFT):
    case str_hash(CROSS_SWITCH_END_LEFT): return end_left;
    case str_hash(CROSS_START_RIGHT):
    case str_hash(CROSS_SWITCH_START_RIGHT): return start_right;
    case str_hash(CROSS_END_RIGHT):
    case str_hash(CROSS_SWITCH_END_RIGHT): return end_right;
    default:
      throw utl::fail("could not determine cross direction from node name {}",
                      node_name);
  }
}

}  // namespace soro::infra
