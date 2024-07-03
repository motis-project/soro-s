#include "soro/infrastructure/graph/construction/set_line.h"

#include <string>
#include <type_traits>

#include "utl/verify.h"

#include "soro/utls/string.h"

#include "soro/infrastructure/graph/construction/get_dir.h"
#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/line.h"
#include "soro/infrastructure/parsers/iss/iss_string_literals.h"

namespace soro::infra {

template <typename Element, typename Direction>
  requires(!std::is_same_v<Element, element>)
void set_line(Element& t, Direction const dir, line::id const line) {
  static_assert(
      std::is_same_v<Direction, typename std::decay_t<Element>::direction>);

  auto const line_idx = detail::get_line_idx(dir);
  utls::sassert(line_idx < t.lines_.size(), "idx not in range");

  t.lines_[line_idx] = line;
}

void set_line(end_element& e, std::string const&, line::id const line) {
  e.lines_.front() = line;
}

void set_line(simple_element& e, std::string const& node_name,
              line::id const line) {
  using enum simple_element::direction;

  switch (utls::str_hash(node_name)) {
    case utls::str_hash(KM_JUMP_START):
    case utls::str_hash(LINE_SWITCH_ZERO): {
      set_line(e, first, line);
      return;
    }

    case utls::str_hash(KM_JUMP_END):
    case utls::str_hash(LINE_SWITCH_ONE): {
      set_line(e, second, line);
      return;
    }

    case utls::str_hash(BORDER): {
      set_line(e, first, line);
      set_line(e, second, line);
      return;
    }

    default: {
      throw utl::fail(
          "unknown node name in set kilometre point in simple element");
    }
  }
}

void set_line(track_element& e, std::string const&, line::id const line) {
  e.lines_.front() = line;
}

void set_line(simple_switch& e, std::string const& node_name,
              line::id const line) {
  set_line(e, get_switch_dir(node_name), line);
}

void set_line(cross& e, std::string const& node_name, line::id const line) {
  set_line(e, get_cross_dir(node_name), line);
}

void set_line(element& e, std::string const& node_name, line::id const line) {
  e.apply([&](auto&& x) { set_line(x, node_name, line); });
}

}  // namespace soro::infra
