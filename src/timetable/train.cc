#include "soro/timetable/train.h"

#include "soro/infrastructure/path/length.h"

namespace soro::tt {

using namespace soro::rs;
using namespace soro::infra;

train_node::train_node(soro::infra::route_node const& rn,
                       sequence_point::optional_ptr sp)
    : infra::route_node{rn},
      sequence_point_{sp.has_value()
                          ? sequence_point::optional_ptr(*sp)
                          : sequence_point::optional_ptr(std::nullopt)} {}

bool train_node::omitted() const {
  return omitted_ || (node_->is(type::HALT) && !sequence_point_.has_value());
}

bool is_halt(utls::unixtime arrival, utls::unixtime departure) {
  return arrival != departure &&
         (arrival != utls::INVALID_TIME || departure != utls::INVALID_TIME);
}

bool stop_time::is_halt() const noexcept {
  return soro::tt::is_halt(arrival_, departure_);
}

FreightTrain train::freight() const { return this->physics_.freight(); }
bool train::is_freight() const { return static_cast<bool>(this->freight()); }

bool train::has_ctc() const { return static_cast<bool>(this->ctc()); };
CTC train::ctc() const { return this->physics_.ctc(); }

si::length train::path_length(infrastructure const& infra) const {
  return get_path_length_from_elements(
      utls::coro_map(this->iterate(infra), [](auto&& rn) { return rn.node_; }));
}

relative_time train::first_departure() const {
  auto const result = sequence_points_.front().departure_;
  utls::sassert(valid(result));
  return result;
}

std::size_t train::total_halts() const {
  return utls::count_if(sequence_points_,
                        [](auto&& sp) { return sp.is_halt(); });
}

bool train::effected_by(speed_limit const& spl) const {
  std::ignore = this->freight();
  return spl.type_ == speed_limit::type::GENERAL_ALLOWED &&
         // ignore speed limits with limit == 0 for now.
         !si::is_zero(spl.limit_) &&
         (spl.effects_ == speed_limit::effects::ALL ||
          spl.effects_ == speed_limit::effects::CONVENTIONAL);
}

node::ptr train::get_start_node(infrastructure const& infra) const {
  if (break_in_) {
    return this->first_interlocking_route(infra).first_node(infra);
  } else {
    return *sequence_points_.front().get_node(freight(), infra);
  }
}

station_route::ptr train::first_station_route(
    infrastructure const& infra) const {
  return break_in_
             ? this->first_interlocking_route(infra).first_sr(infra)
             : infra->station_routes_[sequence_points_.front().station_route_];
}

infra::station_route::ptr train::last_station_route(
    infra::infrastructure const& infra) const {
  return break_out_
             ? this->last_interlocking_route(infra).last_sr(infra)
             : infra->station_routes_[sequence_points_.back().station_route_];
}

interlocking_route const& train::first_interlocking_route(
    infrastructure const& infra) const {
  return infra->interlocking_.routes_[path_.front()];
}

infra::interlocking_route const& train::last_interlocking_route(
    infra::infrastructure const& infra) const {
  return infra->interlocking_.routes_[path_.back()];
}

utls::recursive_generator<route_node> iterate_train(
    train const& t, infrastructure const& infra) {

  if (t.path_.size() == 1) {
    auto const from_sr_id =
        t.break_in_ ? t.first_interlocking_route(infra).station_routes_.front()
                    : t.sequence_points_.front().station_route_;
    auto const from_idx =
        t.break_in_
            ? t.first_interlocking_route(infra).start_offset_
            : *t.sequence_points_.front().get_node_idx(t.freight(), infra);

    auto const to_sr_id =
        t.break_out_ ? t.last_interlocking_route(infra).station_routes_.back()
                     : t.sequence_points_.back().station_route_;
    node::idx const to_idx =
        t.break_out_
            ? t.first_interlocking_route(infra).end_offset_
            : *t.sequence_points_.back().get_node_idx(t.freight(), infra) + 1;

    co_yield t.first_interlocking_route(infra).from_to(from_sr_id, from_idx,
                                                       to_sr_id, to_idx, infra);
    co_return;
  }

  if (t.break_in_) {
    co_yield t.first_interlocking_route(infra).iterate(infra);
  } else {
    utls::sassert(t.sequence_points_.front().is_halt(),
                  "First sequence point in non-breakin train {} is not a halt.",
                  t.id_);

    auto const sr_id = t.sequence_points_.front().station_route_;
    auto const node_idx =
        t.sequence_points_.front().get_node_idx(t.freight(), infra);

    utls::sassert(node_idx.has_value(),
                  "All sequence points in a train must have a value. First "
                  "sequence point in train {} does not.",
                  t.id_);

    co_yield t.first_interlocking_route(infra).from(sr_id, *node_idx, infra);
  }

  for (auto i = 1U; i < t.path_.size() - 1; ++i) {
    auto const& ir = infra->interlocking_.routes_[t.path_[i]];
    // skip the first element, we already yielded it
    // two following interlocking routes share the first and last element
    co_yield ir.from(ir.first_sr_id(), ir.start_offset_ + 1, infra);
  }

  auto const last_ir = t.last_interlocking_route(infra);
  if (t.break_out_) {
    co_yield last_ir.from(last_ir.first_sr_id(), last_ir.start_offset_ + 1,
                          infra);
  } else {
    utls::sassert(t.sequence_points_.back().is_halt(),
                  "Last sequence point in non-breakout train {} is not a halt.",
                  t.id_);

    auto const sr_id = t.sequence_points_.back().station_route_;
    auto const node_idx =
        t.sequence_points_.back().get_node_idx(t.freight(), infra);

    utls::sassert(node_idx.has_value(),
                  "All sequence points in a train must have a value. Last "
                  "sequence point in train {} does not.",
                  t.id_);

    co_yield last_ir.from_to(last_ir.first_sr_id(), last_ir.start_offset_ + 1,
                             sr_id, (*node_idx) + 1, infra);
  }
}

utls::recursive_generator<train_node> train::iterate(
    infrastructure const& infra) const {
  uint32_t sp_idx = 0;
  for (auto&& rn : iterate_train(*this, infra)) {
    if (sp_idx < sequence_points_.size() &&
        *sequence_points_[sp_idx].get_node(freight(), infra) == rn.node_) {

      co_yield train_node(
          rn, sequence_point::optional_ptr(&sequence_points_[sp_idx]));
      ++sp_idx;
    } else {
      co_yield train_node(rn, {});
    }
  }
}

}  // namespace soro::tt
