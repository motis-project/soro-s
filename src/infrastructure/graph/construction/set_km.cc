#include "soro/infrastructure/graph/construction/set_km.h"

#include <string>
#include <type_traits>

#include "utl/verify.h"

#include "soro/utls/string.h"

#include "soro/infrastructure/graph/construction/get_dir.h"
#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/kilometrage.h"
#include "soro/infrastructure/parsers/iss/iss_string_literals.h"

namespace soro::infra {

template <typename Element, typename Direction>
  requires(!std::is_same_v<Element, element>)
void set_km(Element& t, Direction const dir, kilometrage const km) {
  static_assert(
      std::is_same_v<Direction, typename std::decay_t<Element>::direction>);

  auto const km_idx = detail::get_km_idx(dir);
  utls::sassert(km_idx < t.km_.size(), "idx not in range");

  t.km_[km_idx] = km;
}

void set_km(end_element& e, std::string const&, kilometrage const km_point) {
  e.km_.front() = km_point;
}

void set_km(simple_element& e, std::string const& name,
            kilometrage const km_point) {
  using enum simple_element::direction;

  switch (utls::str_hash(name)) {
    case utls::str_hash(KM_JUMP_START):
    case utls::str_hash(LINE_SWITCH_ZERO): {
      set_km(e, first, km_point);
      return;
    }

    case utls::str_hash(KM_JUMP_END):
    case utls::str_hash(LINE_SWITCH_ONE): {
      set_km(e, second, km_point);
      return;
    }

    case utls::str_hash(BORDER): {
      set_km(e, first, km_point);
      set_km(e, second, km_point);
      return;
    }

    default: {
      throw utl::fail(
          "unknown node name in set kilometre point in simple element");
    }
  }
}

void set_km(track_element& e, std::string const&, kilometrage const km_point) {
  e.km_.front() = km_point;
}

void set_km(simple_switch& e, std::string const& name, kilometrage const km) {
  set_km(e, get_switch_dir(name), km);
}

void set_km(cross& e, std::string const& name, kilometrage const km) {
  set_km(e, get_cross_dir(name), km);
}

void set_km(element& e, std::string const& name, kilometrage km) {
  e.apply([&](auto&& x) { set_km(x, name, km); });
}

}  // namespace soro::infra
