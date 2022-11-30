#include "soro/timetable/train.h"

#include "soro/infrastructure/path/length.h"

namespace soro::tt {

using namespace soro::infra;

bool sequence_point::is_halt() const noexcept { return type_ != type::TRANSIT; }
bool sequence_point::is_halt(type const t) const noexcept { return t == type_; }

relative_time sequence_point::transit_time() const noexcept {
  return departure_;
}

bool sequence_point::is_transit() const noexcept {
  return type_ == type::TRANSIT;
}

bool sequence_point::has_transit_time() const noexcept {
  return type_ == type::TRANSIT && valid(departure_);
}

absolute_time sequence_point::absolute_arrival(
    date::year_month_day const departure_day) const noexcept {
  return relative_to_absolute(departure_day, this->arrival_);
}

absolute_time sequence_point::absolute_departure(
    date::year_month_day const departure_day) const noexcept {
  return relative_to_absolute(departure_day, this->departure_);
}

utls::optional<infra::node::ptr> sequence_point::get_node(
    rs::FreightTrain const freight, infra::infrastructure const& infra) const {
  auto const sr = infra->station_routes_[station_route_];

  if (!(is_halt() && has_transit_time())) {
    return {};
  }

  if (has_transit_time()) {
    return sr->get_runtime_checkpoint_node();
  } else {
    utls::sassert(is_halt());
    return sr->get_halt_node(freight);
  }
}

bool is_halt(utls::unixtime arrival, utls::unixtime departure) {
  return arrival != departure &&
         (arrival != utls::INVALID_TIME || departure != utls::INVALID_TIME);
}

bool stop_time::is_halt() const noexcept {
  return soro::tt::is_halt(arrival_, departure_);
}

rs::FreightTrain train::freight() const { return this->physics_.freight(); }
bool train::is_freight() const { return static_cast<bool>(this->freight()); }

bool train::has_ctc() const { return static_cast<bool>(this->ctc()); };
rs::CTC train::ctc() const { return this->physics_.ctc(); }

si::length train::path_length(infrastructure const& infra) const {
  return get_path_length_from_elements(
      utls::coro_map(this->iterate(infra), [](auto&& rn) { return rn.node_; }));
}

relative_time train::first_departure() const {
  auto const result =
      path_.entries_.front().sequence_points_.front().departure_;
  utls::sassert(result != INVALID_RELATIVE_TIME);
  return result;
}

// utls::unixtime train::last_arrival() const {
//   return stop_times_.back().arrival_;
//}

std::size_t train::total_halts() const {
  return utls::accumulate(
      path_.entries_, std::size_t{0}, [](auto&& acc, auto&& entry) {
        return acc + utls::count_if(entry.sequence_points_,
                                    [](auto&& sp) { return sp.is_halt(); });
      });
}

node::ptr train::get_start_node(infrastructure const& infra) const {
  if (this->path_.break_in_) {
    return this->first_interlocking_route(infra).first_node(infra);
  } else {
    utls::sassert(false, "Not implemented");
    return nullptr;
  }
}

// node::ptr train::get_end_node() const {
//   return nullptr;
//   return path_.back()->get_halt(freight_);
//}

station_route::ptr train::first_station_route(
    infrastructure const& infra) const {
  return infra->interlocking_
      .interlocking_routes_[this->path_.entries_.front().interlocking_id_]
      .first_sr(infra);
}

interlocking_route const& train::first_interlocking_route(
    infrastructure const& infra) const {
  return infra->interlocking_
      .interlocking_routes_[this->path_.entries_.front().interlocking_id_];
}

utls::recursive_generator<route_node> train::iterate(
    infrastructure const&) const {
  utls::sassert(false, "Not implemented");

  route_node rn;
  co_yield rn;

  //  if (break_in_) {
  //    this->first_interlocking_route(infra).from
  //    co_yield
  //    infra->interlocking_.interlocking_routes_path_.front()->iterate(skip,
  //    infra);
  //  } else {
  //    co_yield path_.front()->from
  //  }
  //
  //  if (break_in_ || !stops.front().is_halt()) {
  //    co_yield path_.front()->iterate(skip, infra);
  //  } else {
  //    co_yield path_.front()->f
  //  }
  //
  //  for (auto path_idx = 1U; path_idx < path_.size() - 1; ++path_idx) {
  //    co_yield path_[path_idx]->iterate(skip, infra);
  //  }
  //
  //  if (break_out_ || !stops.back().is_halt()) {
  //    co_yield path_.back()->iterate(skip, infra);
  //  } else {
  //    co_yield path_.back()->from(skip, infra);
  //  }
  //
  //  co_yield
  //  this->path_.front()->from_to(path_.front()->get_halt_idx(freight_),
  //                                        path_.front()->size() - 1, skip);
  //
  //  for (auto path_idx = 1U; path_idx < path_.size() - 1; ++path_idx) {
  //    // dont use ->entire() here, since it would co_yield the first and last
  //    // element of every interlocking route twice!
  //    for (auto&& rn :
  //         path_[path_idx]->from_to(0, path_[path_idx]->size() - 1, skip)) {
  //
  //      rn.omitted_ =
  //          !stop_times_[path_idx].is_halt() && rn.node_->is(type::HALT);
  //
  //      if (rn.omitted_ && static_cast<bool>(skip)) {
  //        continue;
  //      }
  //
  //      co_yield rn;
  //    }
  //  }
  //
  //  co_yield this->path_.back()->from_to(
  //      0, path_.back()->get_halt_idx(freight_) + 1, skip);
}

// bool train::has_event_in_interval(utls::unixtime const start,
//                                   utls::unixtime const end) const {
//   return utls::any_of(stop_times_, [&](auto&& st) {
//     return st.arrival_.in_interval(start, end) ||
//            st.departure_.in_interval(start, end);
//   });
// }

}  // namespace soro::tt
