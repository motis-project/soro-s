#include "soro/timetable/parsers/raw_to_trains.h"

#include "utl/to_set.h"

#include "soro/utls/algo/slice.h"
#include "soro/utls/algo/sort_and_intersect.h"
#include "soro/utls/sassert.h"

namespace soro::tt {

using namespace rs;
using namespace infra;

auto get_best_fitting_ssr(soro::vector<ir_ptr> const& candidates,
                          soro::vector<station_route::ptr> const& sr_run,
                          soro::size_type current_sr_idx) {
  ir_ptr chosen_ssr = nullptr;
  soro::size_type prefix_length = 0;

  for (auto const& candidate : candidates) {
    soro::size_type sr_offset = 0;

    while (sr_offset < candidate->station_routes_.size() &&
           current_sr_idx + sr_offset < sr_run.size() &&
           sr_run[current_sr_idx + sr_offset]->id_ ==
               candidate->station_routes_[sr_offset]->id_) {
      ++sr_offset;
    }

    if (sr_offset > prefix_length) {
      chosen_ssr = candidate;
      prefix_length = sr_offset;
    }
  }

  current_sr_idx += prefix_length - 1;

  return std::pair(current_sr_idx, chosen_ssr);
}

auto get_first_ssr(soro::vector<station_route::ptr> const& sr_run,
                   FreightTrain const freight,
                   interlocking_subsystem const& ssr_man,
                   station_route_graph const& srg) {
  auto const& first_sr = sr_run.front();

  auto parti_candidates = ssr_man.sr_to_participating_irs_[first_sr->id_];
  auto stop_candidates =
      ssr_man.halting_at_.at(sr_run.front()->get_halt_node(freight)->id_);

  auto const& candidates =
      utls::sort_and_intersect(parti_candidates, stop_candidates);

  if (srg.predeccesors_[first_sr->id_].empty()) {
    return get_best_fitting_ssr(candidates, sr_run, 0);
  }

  ir_ptr chosen_ssr = nullptr;

  soro::size_type prefix_length = 0;

  for (auto const& candidate : candidates) {
    soro::size_type sr_offset = 0;

    auto parti_it = utls::find(candidate->station_routes_, first_sr);

    while (parti_it != std::end(candidate->station_routes_) &&
           (sr_run[0 + sr_offset]->id_ == (*parti_it)->id_)) {

      ++sr_offset;
      std::advance(parti_it, 1);
    }

    if (sr_offset > prefix_length) {
      chosen_ssr = candidate;
      prefix_length = sr_offset;
    }
  }

  return std::pair(prefix_length - 1, chosen_ssr);
}

auto get_ssr_run_path(soro::vector<station_route::ptr> const& sr_run,
                      FreightTrain const freight,
                      interlocking_subsystem const& ssr_man,
                      station_route_graph const& srg) {
  soro::vector<ir_ptr> ssr_path;

  auto [current_sr_idx, chosen_ssr] =
      get_first_ssr(sr_run, freight, ssr_man, srg);
  ssr_path.emplace_back(chosen_ssr);
  node_ptr current_node = ssr_path.back()->nodes().back();

  while (current_sr_idx != sr_run.size() - 1) {

    // TODO(julian) this should become unnecessary when all train paths
    // contained in a timetable are parsed correctly
    auto const can_it = ssr_man.starting_at_.find(current_node->id_);
    if (can_it == std::end(ssr_man.starting_at_)) {
      ssr_path.clear();
      return ssr_path;
    }

    auto const& candidates = can_it->second;

    std::tie(current_sr_idx, chosen_ssr) =
        get_best_fitting_ssr(candidates, sr_run, current_sr_idx);

    // TODO(julian) this should become unnecessary when all train paths
    // contained in a timetable are parsed correctly
    if (chosen_ssr == nullptr) {
      ssr_path.clear();
      return ssr_path;
    }

    ssr_path.emplace_back(chosen_ssr);
    current_node = ssr_path.back()->nodes().back();
  }

  auto const& last_sr = sr_run[current_sr_idx];

  while (true) {
    auto const& candidate_it = ssr_man.starting_at_.find(current_node->id_);

    if (candidate_it == std::cend(ssr_man.starting_at_)) {
      break;
    }

    auto chosen_it = utls::find_if(candidate_it->second, [&](ir_ptr can) {
      return can->station_routes_.front()->id_ == last_sr->id_;
    });

    if (chosen_it == std::end(candidate_it->second)) {
      break;
    }

    chosen_ssr = *chosen_it;

    bool const found_internal = chosen_ssr->station_routes_.size() == 1;
    ssr_path.emplace_back(chosen_ssr);

    if (!found_internal) {
      break;
    }

    current_node = ssr_path.back()->nodes().back();
  }

  return ssr_path;
}

train_physics get_train_physics(raw_train::characteristic const& rc,
                                rolling_stock const& rolling_stock) {
  auto tvs = soro::to_vec(rc.traction_units_, [&](auto&& tv) {
    return rolling_stock.get_traction_vehicle(tv.series_, tv.owner_,
                                              tv.variant_);
  });

  return {tvs, rc.carriage_weight_, rc.length_, rc.max_speed_};
}

soro::vector<soro::unique_ptr<train>> raw_trains_to_trains(
    std::vector<raw_train> const& raw_trains,
    infra::interlocking_subsystem const& ssr_man,
    infra::station_route_graph const& srg, rolling_stock const& rolling_stock) {
  soro::vector<soro::unique_ptr<train>> result;

  for (auto const& rt : raw_trains) {
    auto tr = soro::make_unique<train>();

    tr->id_ = rt.id_;
    tr->name_ = rt.name_;
    tr->freight_ = rt.freight_;
    tr->ctc_ = rt.ctc_;
    tr->physics_ = get_train_physics(rt.charac_, rolling_stock);

    // TODO(julian) for now only a single station route per station
    assert(rt.run_.routes_.size() == rt.run_.arrivals_.size());
    assert(rt.run_.arrivals_.size() == rt.run_.departures_.size());
    assert(rt.run_.departures_.size() == rt.run_.min_stop_times_.size());

    tr->path_ = get_ssr_run_path(rt.run_.routes_, rt.freight_, ssr_man, srg);

    tr->stop_times_.resize(tr->path_.size());

    for (auto const [route, arr, dep, min_stop_time] : rt.run_.entries()) {
      if (!is_halt(arr, dep)) {
        continue;
      }

      auto const idx = utls::find_if_position(
          std::cbegin(tr->path_), std::cend(tr->path_),
          [&, r = route](ir_ptr const ssr_ptr) {
            return utls::contains(ssr_ptr->nodes(),
                                  r->get_halt_node(rt.freight_));
          });

      tr->stop_times_[idx] = {
          .arrival_ = arr, .departure_ = dep, .min_stop_time_ = min_stop_time};
    }

    utls::sassert(
        utls::count_if(
            utl::zip(rt.run_.arrivals_, rt.run_.departures_),
            [](auto&& p) { return is_halt(std::get<0>(p), std::get<1>(p)); }) ==
            utls::count_if(tr->stop_times_,
                           [](auto&& st) { return st.is_halt(); }),
        "Every halt in the train run must be present in the signal station "
        "route run");

    // TODO(julian) every station can only appear once in a train run
    utls::sassert(utl::to_set(
                      rt.run_.routes_,
                      [](auto&& route) {
                        return route->station_->id_;
                      }).size() == rt.run_.routes_.size(),
                  "For now every station can only appear once in a train run");

    // remove every emtpy trailing ssr halt entry and the corresponding ssr
    // path entry
    auto const last_halt_idx = utls::reverse_find_if_position(
        tr->stop_times_, [](auto&& st) { return st.is_halt(); });

    utls::slice(tr->path_, 0, last_halt_idx + 1);
    utls::slice(tr->stop_times_, 0, last_halt_idx + 1);

    result.emplace_back(std::move(tr));
  }

  return result;
}

}  // namespace soro::tt