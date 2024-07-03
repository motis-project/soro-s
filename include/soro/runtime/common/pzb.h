#pragma once

#include "soro/timetable/train.h"

#include "soro/runtime/driver/command.h"

namespace soro::runtime {

struct runtime_commands {
  train_state initial_;
  std::vector<command> commands_;
};

si::time time_under_s(train_state current, command const& command,
                      si::speed const v) {
  auto result = si::time::zero();

  switch (command.action_) {
    case command::action::brake: {
      break;
    }

    case command::action::cruise: {
      result +=
          command.speed_ < v ? command.time_ - current.time_ : si::time::zero();
      break;
    }

    case command::action::halt: {
      result += command.time_ - current.time_;
      break;
    };

      // TODO(julian) not implemented
    case command::action::coast:
    case command::action::invalid:
    case command::action::accelerate: break;
  }

  current = command;

  return result;
}

struct pzb {
  // we will ignore the pzb magnets with a frequency of 2000hz for now
  enum class magnet : uint8_t { hz1000, hz500, hz2000 };

  enum class train_type : uint8_t {
    U,  // untere, lower
    M,  // mittlere, middle
    O  // obere, upper
  };

  enum class state : uint8_t {
    free,
    hz1000_braking,
    hz1000_v1,
    hz1000_restriction,
    // hz500_braking,
    // hz500_v1,
    // hz500_restriction,
    invalid
  };

  auto constexpr hz1000_switch_speed = si::from_km_h(10);
  auto constexpr hz1000_switch_time = si::from_s(15);

  pzb() = default;
  pzb(state const s, si::length const dist)
      : current_state_{s},
        hz1000_dist_{dist},
        time_under_switch_speed_{si::time::zero()} {}

  train_type get_train_type(tt::train const& t) const {
    if (t.physics_.brake_position() == "G") {
      return train_type::U;
    }

    if (t.physics_.percentage() <= rs::brake_weight_percentage{65}) {
      return train_type::U;
    } else if (t.physics_.percentage() <= rs::brake_weight_percentage{110}) {
      return train_type::M;
    } else {  // > 110
      return train_type::O;
    }
  }

  si::speed get_hz1000_v1(tt::train const& t) const {
    switch (get_train_type(t)) {
      case train_type::O: return si::from_km_h(85);
      case train_type::M: return si::from_km_h(70);
      case train_type::U: return si::from_km_h(55);
    }
  }

  si::speed get_hz1000_v2(tt::train const&) const { return si::from_km_h(45); }

  si::accel get_braking_accel(tt::train const& t) const {
    switch (get_train_type(t)) {
      case train_type::O: return si::from_m_s2(-0.97);
      case train_type::M: return si::from_m_s2(-0.53);
      case train_type::U: return si::from_m_s2(-0.36);
    }
  }

  si::speed current_limit(si::length const dist, si::time const time) const {
    switch (current_state_) {
      case state::free: {
      }

      case state::hz1000_braking: {
        break;
      }

      case state::hz1000_v1: {
        break;
      }

      case state::hz1000_restriction: {
        break;
      }

      case state::invalid: {
        throw utl::fail("invalid state");
      }
    }
  }

  void pass_magnet(magnet const m) {
    utls::sassert(m == magnet::hz1000, "only hz1000 magnets are supported");

    switch (current_state_) {
      case state::free: {
        current_state_ = state::hz1000_braking;
        break;
      }

      case state::hz1000_v1:
      case state::hz1000_braking:
      case state::hz1000_restriction:
      case state::invalid: throw utl::fail("invalid state");
    }
  }

  // minimum distance after passing a 1000hz magnet required to release the pzb
  constexpr auto min_release_dist = si::from_m(700);

  void release(si::length const dist) {
    utls::expect(dist > hz1000_dist_, "release before 1000hz");

    if (dist - hz1000_dist_ < min_release_dist) {
      throw utl::fail("release too early");
    }

    switch (current_state_) {
      case state::hz1000_v1:
      case state::hz1000_braking:
      case state::hz1000_restriction: {
        current_state_ = state::free;
        break;
      }

      case state::free: throw utl::fail("release in free state");
      case state::invalid: throw utl::fail("invalid state");
    }
  }

  void add_command(train_state const& initial, command const& c) {
    time_under_switch_speed_ += time_under_s(initial, c, hz1000_switch_speed);

    switch (current_state_) {
      case state::hz1000_v1: {
        break;
      }

      case state::hz1000_braking: {
        if (time_under_switch_speed_ > hz1000_switch_time) {
          current_state_ = state::hz1000_restriction;
        }

        break;
      }

      case state::hz1000_restriction: {
        break;
      }

      case state::free:
      case state::invalid: throw utl::fail("invalid state");
    }
  }

  state current_state_{state::invalid};
  si::time time_under_switch_speed_{si::time::invalid()};

  si::length hz1000_dist_{si::length::invalid()};  // approach
  si::length hz500_dist_{si::length::invalid()};  // ~250m before main
  si::length hz2000_dist_{si::length::invalid()};  // main
};

}  // namespace soro::runtime