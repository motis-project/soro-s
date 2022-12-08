#include "soro/runtime/interval.h"

#include "soro/utls/sassert.h"

#include "soro/runtime/runtime_physics.h"

namespace soro::runtime {

using namespace soro::si;
using namespace soro::tt;
using namespace soro::utls;
using namespace soro::infra;

si::speed get_initial_spl(train const& train, infrastructure const& infra) {
  auto const start =
      train.sequence_points_.front().get_node_idx(train.freight(), infra);

  utls::sassert(start.has_value(), "First Route does not have an halt.");

  for (node::idx idx = start.value(); idx > 0; --idx) {
    auto const node = train.first_station_route(infra)->nodes(idx);

    if (node->is(type::SPEED_LIMIT)) {
      auto const& spl = infra->graph_.element_data<speed_limit>(node);
      if (train.effected_by(spl)) {
        utls::sassert(si::valid(spl.limit_));
        return spl.limit_;
      }
    }
  }

  auto curr_node = train.first_station_route(infra)->nodes().front();
  while (curr_node->reverse_ahead() != nullptr) {
    if (curr_node->is(type::SPEED_LIMIT)) {
      auto const& spl = infra->graph_.element_data<speed_limit>(curr_node);
      if (train.effected_by(spl)) {
        utls::sassert(si::valid(spl.limit_));
        return spl.limit_;
      }
    }

    curr_node = curr_node->reverse_ahead();
  }

  return infra->defaults_.stationary_speed_limit_;
}

bool adjust_speed_limits(interval_list& list, rs::train_physics const& tp) {
  bool had_to_adjust = false;

  //  adjust top speed if it is too high for brake path length
  for (auto idx = 1U; idx < list.size(); ++idx) {
    auto& prev_interval = list[idx - 1];
    auto& interval = list[idx];

    auto const initial_speed = std::min(interval.limit_left_, tp.max_speed());
    if (interval.limit_right_ >= initial_speed && !interval.is_halt()) {
      continue;
    }

    auto const target_speed = interval.target_speed(tp);
    auto const& deaccel_results = brake(tp, initial_speed, target_speed);

    auto const braking_path_length = deaccel_results.back().distance_;
    auto const interval_length = interval.distance_ - prev_interval.distance_;

    if (interval_length >= braking_path_length) {
      continue;
    }

    had_to_adjust = true;

    si::speed new_speed = initial_speed - from_m_s(1.0);
    while (new_speed > target_speed &&
           brake(tp, new_speed, target_speed).back().distance_ >=
               interval_length) {
      new_speed -= from_m_s(1.0);
    }

    if (new_speed <= target_speed) {
      new_speed = target_speed;
    }

    utl::verify(new_speed < initial_speed,
                "New speed limit should be smaller than old speed limit");
    utl::verify(new_speed > si::ZERO<si::speed>, "New speed should not be 0!");

    interval.limit_left_ = new_speed;
    prev_interval.limit_right_ = new_speed;
  }

  return had_to_adjust;
}

type_set const BORDER_TYPES{
    type::MAIN_SIGNAL, type::HALT
    /* type::SLOPE */  // not yet supported

    // only when the speed_limit::poa is HERE and train is effected
    /* type::SPEED_LIMIT, */
};

bool is_interval_border(node_ptr const node, train const& train,
                        infrastructure const& infra) {

  if (BORDER_TYPES.contains(node->type())) {
    return true;
  }

  if (node->is(type::SPEED_LIMIT)) {
    auto const& spl = infra->graph_.element_data<speed_limit>(node);
    return spl.poa_ == speed_limit::poa::HERE && train.effected_by(spl);
  }

  return false;
}

struct speed_limit_duo {
  speed_limit::optional_ptr here_;
  speed_limit::optional_ptr last_signal_;
};

speed_limit_duo get_speed_limit(train_node const& tn, train const& train,
                                infrastructure const& infra) {
  speed_limit_duo result;

  auto const handle_spl = [&train, &result](speed_limit const* spl) {
    if (!train.effected_by(*spl)) {
      return;
    }

    spl->poa_ == speed_limit::poa::HERE ? result.here_ = spl
                                        : result.last_signal_ = spl;
  };

  // the extra speed limit overwrites the general speed limit if necessary

  if (tn.node_->is(type::SPEED_LIMIT)) {
    auto const& spl = infra->graph_.element_data<speed_limit>(tn.node_);
    handle_spl(&spl);
  }

  if (tn.extra_spl_.has_value()) {
    handle_spl(*tn.extra_spl_);
  }

  return result;
}

auto insert_last_signal_spl(si::speed const new_limit, interval_list& list,
                            soro::size_t last_ms_idx) {

  if (last_ms_idx >= list.size()) {
    return list.size();
  }

  auto const replaced_limit = list.at(last_ms_idx).limit_right_;

  while (last_ms_idx < list.size() &&
         list.at(last_ms_idx).limit_right_ == replaced_limit) {
    list.at(last_ms_idx).limit_right_ = new_limit;

    if (last_ms_idx < list.size() - 1) {
      list.at(last_ms_idx + 1).limit_left_ = new_limit;
    }
    ++last_ms_idx;
  }

  return last_ms_idx;
}

interval_list get_interval_list(train const& train, type_set const& event_types,
                                infrastructure const& infra) {
  utls::sassert(!train.break_out_ && !train.break_in_, "Not supported yet");

  interval_list list;

  length current_distance = ZERO<length>;

  element::ptr prev_element = train.get_start_node(infra)->element_;

  soro::size_t last_ms = 0;

  si::speed prev_limit = si::INVALID<si::speed>;
  si::speed current_limit = get_initial_spl(train, infra);

  std::vector<event> current_events;

  for (auto const& tn : train.iterate(infra)) {
    current_distance += tn.node_->element_->get_distance(prev_element);
    prev_element = tn.node_->element_;

    if (tn.omitted()) {
      continue;
    }

    auto const spl_duo = get_speed_limit(tn, train, infra);

    if (spl_duo.last_signal_.has_value()) {
      auto const ms_limit = (*spl_duo.last_signal_)->limit_;
      auto const updated_to = insert_last_signal_spl(ms_limit, list, last_ms);
      if (updated_to == list.size()) {
        prev_limit = ms_limit;
        current_limit = ms_limit;
      }
    }

    if (spl_duo.here_.has_value()) {
      prev_limit = current_limit;
      current_limit = (*spl_duo.here_)->limit_;
    }

    if (tn.node_->is(type::MAIN_SIGNAL)) {
      last_ms = list.size();
    }

    if (event_types.contains(tn.node_->type())) {
      current_events.emplace_back(tn.node_, current_distance);
    }

    if (BORDER_TYPES.contains(tn.node_->type()) || spl_duo.here_.has_value()) {
      auto const use_sp =
          tn.sequence_point_.has_value() && (*tn.sequence_point_)->is_halt();

      interval const new_interval(current_distance, prev_limit, current_limit,
                                  use_sp ? tn.sequence_point_ : nullptr,
                                  current_events);

      if (!list.empty() && list.back().distance_ == current_distance) {
        list.back().append(new_interval);
      } else {
        list.push_back(new_interval);
      }

      prev_limit = current_limit;
      current_events = {};
    }
  }

  while (adjust_speed_limits(list, train.physics_))
    ;

  return list;
}

}  // namespace soro::runtime
