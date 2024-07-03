#include "soro/infrastructure/graph/construction/set_neighbour.h"

#include <string>

#include "soro/utls/sassert.h"
#include "soro/utls/string.h"

#include "soro/infrastructure/graph/construction/get_dir.h"
#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/section.h"
#include "soro/infrastructure/kilometrage.h"
#include "soro/infrastructure/parsers/iss/iss_string_literals.h"

namespace soro::infra {

using namespace utls;

void set_neighbour(end_element& e, std::string const&, section::position const,
                   element* neigh, mileage_dir const dir) {
  detail::set_neighbour(e, dir, end_element::direction::oneway, neigh);
}

void set_neighbour(simple_element& e, std::string const& name,
                   section::position const pos, element* neigh,
                   mileage_dir const dir) {
  using enum mileage_dir;
  using enum simple_element::direction;

  utls::expect(pos != section::position::middle, "simple element can't be mid");

  switch (str_hash(name)) {
    case str_hash(KM_JUMP_START):
    case str_hash(LINE_SWITCH_ZERO):
      detail::set_neighbour(e, dir, first, neigh);
      return;

    case str_hash(KM_JUMP_END):
    case str_hash(LINE_SWITCH_ONE):
      detail::set_neighbour(e, dir, second, neigh);
      return;

    case str_hash(BORDER): {
      detail::set_neighbour(
          e, dir, pos == section::position::end ? first : second, neigh);
    }
      return;
    default: {
      sassert(false, "should not be reachable");
    }
  }
}

void set_neighbour(simple_switch& e, std::string const& name,
                   section::position const, element* neigh,
                   mileage_dir const dir) {
  detail::set_neighbour(e, dir, get_switch_dir(name), neigh);
}

void set_neighbour(track_element& e, std::string const&,
                   section::position const, element* neigh,
                   mileage_dir const dir) {
  using enum mileage_dir;
  using enum track_element::direction;

  detail::set_neighbour(e, dir, oneway, neigh);
  //  (is_rising(dir) ? e.neighbour(ahead) : e.neighbour(behind)) = neigh;
}

void set_neighbour(cross& e, std::string const& name, section::position const,
                   element* neigh, mileage_dir const dir) {
  detail::set_neighbour(e, dir, get_cross_dir(name), neigh);

  //  switch (str_hash(name)) {
  //    case str_hash(CROSS_START_LEFT):
  //    case str_hash(CROSS_SWITCH_START_LEFT): {
  //      (is_rising(dir) ? neighbour(e, rising, start_left)
  //                      : neighbour(e, falling, start_left)) = neigh;
  //      return;
  //    }
  //    case str_hash(CROSS_END_LEFT):
  //    case str_hash(CROSS_SWITCH_END_LEFT): {
  //      (is_rising(dir) ? neighbour(e, rising, end_left)
  //                      : neighbour(e, falling, end_left)) = neigh;
  //      return;
  //    }
  //    case str_hash(CROSS_START_RIGHT):
  //    case str_hash(CROSS_SWITCH_START_RIGHT): {
  //      (is_rising(dir) ? neighbour(e, rising, start_right)
  //                      : neighbour(e, falling, start_right)) = neigh;
  //      return;
  //    }
  //    case str_hash(CROSS_END_RIGHT):
  //    case str_hash(CROSS_SWITCH_END_RIGHT): {
  //      (is_rising(dir) ? e.neighbour(rising, end_right)
  //                      : e.neighbour(falling, end_right)) = neigh;
  //      return;
  //    }
  //  }
}

void set_neighbour(element& e, std::string const& name,
                   section::position const pos, element* neigh,
                   mileage_dir const dir) {
  e.apply([&](auto&& x) { set_neighbour(x, name, pos, neigh, dir); });
}

}  // namespace soro::infra
