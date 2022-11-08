#include "soro/runtime/interval.h"

#include "soro/utls/serror.h"

#include "soro/runtime/runtime_physics.h"

namespace soro::runtime {

using namespace soro::si;
using namespace soro::tt;
using namespace soro::utls;
using namespace soro::infra;

// TODO(julian) pass train here for checking etcs capabilities and so forth
bool applies(speed_limit const& spl /*, tt::train const& */) {
  return spl.type_ == speed_limit::type::GENERAL_ALLOWED &&
         (spl.effects_ == speed_limit::effects::ALL ||
          spl.effects_ == speed_limit::effects::CONVENTIONAL);
}

speed get_initial_spl(station_route const& first_route,
                      infrastructure const& infra,
                      rs::FreightTrain const freight) {
  speed_limit last_spl;

  auto const start = first_route.get_halt_idx(freight);

  for (node::idx idx = start; idx > 0; --idx) {
    auto const node = first_route.nodes(idx);

    if (node->is(type::SPEED_LIMIT)) {
      auto const& spl = infra->graph_.element_data<speed_limit>(node);
      if (applies(spl)) {
        last_spl = spl;
        break;
      }
    }
  }

  if (valid(last_spl.limit_)) {
    return last_spl.limit_;
  }

  auto curr_node = first_route.nodes().front();
  while (curr_node->reverse_ahead() != nullptr) {
    if (curr_node->is(type::SPEED_LIMIT)) {
      auto const& spl = infra->graph_.element_data<speed_limit>(curr_node);
      if (applies(spl)) {
        last_spl = spl;
        break;
      }
    }

    curr_node = curr_node->reverse_ahead();
  }

  if (valid(last_spl.limit_)) {
    return last_spl.limit_;
  }

  return infra->defaults_.stationary_speed_limit_;
}

void propagate_speed_limits(interval_list& list) {
  // propagate speed limits along the list
  auto current_spl = list.front().speed_limit_;
  for (auto& interval : list) {
    if (!valid(interval.speed_limit_)) {
      interval.speed_limit_ = current_spl;
      continue;
    }

    if (interval.speed_limit_ != current_spl) {
      current_spl = interval.speed_limit_;
      continue;
    }
  }
}

bool adjust_speed_limits(interval_list& list, rs::train_physics const& tv) {
  bool had_to_adjust = false;

  //  adjust top speed if it is too high for brake path length
  for (auto idx = 1UL; idx < list.size() - 1; ++idx) {
    auto const& prev_interval = list[idx - 1];
    auto& interval = list[idx];
    auto const& next_interval = list[idx + 1];

    auto const initial_speed = std::min(interval.speed_limit_, tv.max_speed());
    if (!(next_interval.speed_limit_ < initial_speed || interval.halt_)) {
      continue;
    }

    auto const target_speed =
        interval.halt_ ? ZERO<speed> : next_interval.speed_limit_;

    auto const& deaccel_results = brake(tv, initial_speed, target_speed);

    auto const braking_path_length = deaccel_results.back().distance_;
    auto const interval_length = interval.distance_ - prev_interval.distance_;

    if (interval_length >= braking_path_length) {
      continue;
    }

    had_to_adjust = true;

    speed new_speed = initial_speed - from_m_s(1.0);
    while (new_speed > target_speed &&
           brake(tv, new_speed, target_speed).back().distance_ >=
               interval_length) {
      new_speed -= from_m_s(1.0);
    }

    if (new_speed <= target_speed) {
      new_speed = target_speed;
    }

    utl::verify(new_speed < initial_speed,
                "New speed limit should be smaller than old speed limit");
    utl::verify(new_speed > ZERO<speed>, "New speed should not be 0!");

    interval.speed_limit_ = new_speed;
  }

  return had_to_adjust;
}

bool is_interval_border(node_ptr const node, infrastructure const& infra,
                        type_set const& border_types) {
  return border_types.contains(node->type()) &&
         (!node->is(type::SPEED_LIMIT) ||
          applies(infra->graph_.element_data<speed_limit>(node)));
}

interval_list get_interval_list(train const& tr, type_set const& event_types,
                                type_set const& border_types,
                                infrastructure const& infra) {
  interval_list list;
  list.emplace_back(INVALID<length>, INVALID<speed>);

  length current_distance = ZERO<length>;

  element_ptr prev_element = tr.get_start_node()->element_;
  for (auto const& rn : tr.iterate(skip_omitted::OFF)) {
    current_distance += rn.node_->element_->get_distance(prev_element);
    prev_element = rn.node_->element_;

    if (rn.omitted_) {
      continue;
    }

    bool const same_spot = valid(list.back().distance_) &&
                           list.back().distance_ == current_distance;
    auto const interval_idx = same_spot ? list.size() - 2 : list.size() - 1;

    serror(same_spot && list.size() == 1,
           "Trying to access the element before the last element in a list of "
           "size smaller than 2");

    if (event_types.contains(rn.node_->type())) {
      list[interval_idx].events_.emplace_back(rn.node_, current_distance);
    }

    if (is_interval_border(rn.node_, infra, border_types)) {
      list[interval_idx].elements_.push_back(rn.node_->type());
      list[interval_idx].halt_ |= rn.node_->is(type::HALT);
      list[interval_idx].distance_ = current_distance;

      if (rn.node_->is(type::SPEED_LIMIT)) {
        list[interval_idx].elements_.emplace_back(type::SPEED_LIMIT);
      }

      list.emplace_back(INVALID<length>, INVALID<speed>);
    }

  }

  utl::verify(!valid(list.back().distance_) && list.back().elements_.empty(),
              "Last interval should be empty");
  list.back().distance_ = list[list.size() - 2].distance_;

  list.front().speed_limit_ = get_initial_spl(
      *tr.path_.front()->station_routes_.front(), infra, tr.freight_);

  propagate_speed_limits(list);
  while (adjust_speed_limits(list, tr.physics_))
    ;

  return list;
}

}  // namespace soro::runtime
