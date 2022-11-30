#pragma once

#include "soro/timetable/timetable_options.h"
#include "soro/timetable/train.h"

namespace soro::tt {

struct base_timetable {
  size_t size() const;
  constexpr auto begin() const { return std::cbegin(trains_); }
  constexpr auto end() const { return std::cend(trains_); }

  train const& operator[](train::id id) const;
  train const& operator[](std::string const& name) const;

  utls::unixtime start_;
  utls::unixtime end_;

  soro::vector<train::ptr> trains_;
  soro::map<train::number, train::ptr> number_to_train_;

  soro::vector<soro::unique_ptr<train>> train_store_;
};

base_timetable make_base_timetable(soro::vector<soro::unique_ptr<train>>,
                                   timetable_options const& opts);

}  // namespace soro::tt
