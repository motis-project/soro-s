#pragma once

#include "soro/base/soro_types.h"
#include "soro/si/units.h"
#include "soro/utls/container/optional.h"

namespace soro::infra {

// fwd declaration for speed_limit
struct node;
struct element;

struct empty {};

struct eotd {
  bool signal_{false};
};

struct main_signal {
  soro::string name_;
};

struct slope {
  si::per_mille rising_{si::INVALID<si::per_mille>};
  si::per_mille falling_{si::INVALID<si::per_mille>};
};

struct halt {
  soro::string name_{};
  soro::string identifier_operational_{};
  soro::string identifier_extern_{};

  bool is_passenger_{false};
  bool is_left_{false};
  bool is_right_{false};
};

// speed limit has three different valid combinations:
//   - speed limit point of action is HERE and length_ is not 0:
//     the speed limit is in effect from the specified point and for the
//     specified length
//   - speed limit point of action is HERE and length_ is 0:
//     the speed limit is in effect from the specified point and until the next
//     relevant speed limit
//   - speed limit point of action is LAST_SIGNAL and length_ is 0:
//     speed limit is in effect from the last signal and until the next relevant
//     speed limit

// the combination LAST_SIGNAL and length_ not 0 is not valid

struct speed_limit {
  using ptr = soro::ptr<speed_limit>;
  using optional_ptr = utls::optional<ptr, nullptr>;

  enum class type : uint8_t {
    END_SPECIAL,
    GENERAL_ALLOWED,
    SPECIAL_ALLOWED,
    GENERAL_DIFFERING,
    SPECIAL_DIFFERING,
    INVALID
  };

  // point of activation
  enum class poa : bool { HERE, LAST_SIGNAL };

  enum class effects : uint8_t { ALL, CONVENTIONAL, LZB_ETCS, INVALID };

  // TODO(julian) special speed limit features

  type type_{type::INVALID};
  effects effects_{effects::INVALID};
  bool calculated_{false};

  si::speed limit_{si::INVALID<si::speed>};
  poa poa_{poa::HERE};
  si::length length_{si::INVALID<si::length>};
  soro::ptr<element> element_{nullptr};
  soro::ptr<node> node_{nullptr};
};

struct switch_data {
  soro::string name_;
  cista::optional<soro::string> ui_identifier_;
};

using element_data_t = soro::variant<empty, eotd, slope, halt, speed_limit,
                                     main_signal, switch_data>;

}  // namespace soro::infra
