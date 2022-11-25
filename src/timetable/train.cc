#include "soro/timetable/train.h"

#include "soro/infrastructure/path/length.h"

namespace soro::tt {

using namespace soro::infra;

bool is_halt(utls::unixtime arrival, utls::unixtime departure) {
  return arrival != departure &&
         (arrival != utls::INVALID_TIME || departure != utls::INVALID_TIME);
}

bool stop_time::is_halt() const noexcept {
  return soro::tt::is_halt(arrival_, departure_);
}

bool train::freight() const { return freight_ == rs::FreightTrain::YES; }

bool train::ctc() const { return ctc_ == rs::CTC::YES; }

si::length train::path_length(infrastructure const& infra) const {
  return get_path_length_from_elements(
      utls::coro_map(this->iterate(skip_omitted::OFF, infra),
                     [](auto&& rn) { return rn.node_; }));
}

utls::unixtime train::first_departure() const {
  return stop_times_.front().departure_;
}

utls::unixtime train::last_arrival() const {
  return stop_times_.back().arrival_;
}

std::size_t train::total_halts() const {
  return utls::count_if(stop_times_, [](auto&& st) { return st.is_halt(); });
}

infra::node::ptr train::get_start_node() const {
  utls::sassert(false, "Not implemented");
  return nullptr;
  //  return path_.front()->get_halt(freight_);
}

infra::node::ptr train::get_end_node() const {
  utls::sassert(false, "Not implemented");
  return nullptr;
  //  return path_.back()->get_halt(freight_);
}

infra::station_route::ptr train::first_station_route(
    infra::infrastructure const& infra) const {
  return infra->interlocking_.interlocking_routes_[this->path_.front()]
      .first_sr(infra);
}

utls::recursive_generator<infra::route_node> train::iterate(
    skip_omitted const, infrastructure const&) const {
  utls::sassert(false, "Not implemented");
  route_node rn;
  co_yield rn;

  //  if (break_in_) {
  //    co_yield path_.front()->iterate(skip, infra);
  //  } else {
  //    co_yield path_.front()->from
  //
  //  }
  //
  //  if (break_in_ || !stops.front().is_halt()) {
  //    co_yield path_.front()->iterate(skip, infra);
  //  } else {
  //    co_yield path_.front()->f
  //  }
  //
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

bool train::has_event_in_interval(utls::unixtime const start,
                                  utls::unixtime const end) const {
  return utls::any_of(stop_times_, [&](auto&& st) {
    return st.arrival_.in_interval(start, end) ||
           st.departure_.in_interval(start, end);
  });
}

}  // namespace soro::tt
