#pragma once

#include "soro/infrastructure/station/station.h"
#include "soro/timetable/train.h"

namespace soro::tt {

struct connection {
  enum class mode : uint8_t {
    invalid,
    continuation,
    continuation_with_break,
    uturn,
    uturn_with_break,
    through,
    through_with_break,
    circular,
    circular_with_break,
    park,
    connection,
    trim,
    tact,
    alternative,
    exclusion
  };

  train::number first_train_number_;
  train::number second_train_number_;

  train::id first_train_{train::invalid()};
  train::id second_train_{train::invalid()};

  infra::station::id first_station_{infra::station::invalid()};
  soro::optional<infra::station::id> second_station_{};

  soro::optional<duration> time_;

  mode mode_{mode::invalid};
};

}  // namespace soro::tt