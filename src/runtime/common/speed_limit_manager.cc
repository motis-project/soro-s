#include "soro/runtime/common/speed_limit_manager.h"

#include <algorithm>
#include <iterator>
#include <optional>

#include "utl/erase_if.h"
#include "utl/verify.h"

#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/all_of.h"
#include "soro/utls/std_wrapper/find_if.h"
#include "soro/utls/std_wrapper/min.h"
#include "soro/utls/std_wrapper/min_element.h"

#include "soro/si/units.h"

#include "soro/infrastructure/graph/element_data.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/infrastructure_t.h"

#include "soro/timetable/train.h"

namespace soro::runtime {

using namespace soro::infra;
using namespace soro::tt;

active_speed_limit::active_speed_limit(si::length const dist,
                                       si::length const original_dist,
                                       speed_limit::ptr const spl)
    : dist_{dist}, original_dist_{original_dist}, limit_{spl} {
  utls::expect(limit_->ends_special() || speed().is_valid());
  utls::expect(limit_->ends_special() || !speed().is_zero());
  utls::expect(limit_->ends_special() || !speed().is_infinity());
}

active_speed_limit::active_speed_limit(speed_limit::ptr const spl,
                                       si::length const dist)
    : active_speed_limit(dist, dist, spl) {}

bool active_speed_limit::operator<(auto const& o) const {
  return this->speed() < o.speed();
}

si::speed active_speed_limit::speed() const { return limit_->limit_; }

void speed_limit_manager::add_initial_speed_limit(train const& train,
                                                  speed_limit::ptr const spl,
                                                  si::length const dist) {
  if (spl->from_last_signal()) return;
  if (!train.affected_by(*spl)) return;

  utls::sassert(spl->from_here(), "can only handle 'from here' speed limits");

  auto const is_g1 = spl->is_general();
  if (is_g1 &&
      (!wirkende_g1_.has_value() ||
       (wirkende_g1_->dist_ == dist && wirkende_g1_->speed() > spl->limit_))) {
    wirkende_g1_.emplace(spl, dist);
    return;
  }

  if (spl->begins_special()) {
    auto const same_specialty = [&](auto&& aspl) {
      return aspl.limit_->same_specialties(*spl);
    };

    // we encountered a speed limit before that ends this speed limit
    auto const end = utls::find_if(initial_special_ends_, same_specialty);
    // we already got a speed limit with the same specialty
    auto const second = utls::find_if(g2s_, same_specialty);

    // use the speed limit if it has no fitting end and has not been
    // overwritten
    if (end == std::end(initial_special_ends_) && second != std::end(g2s_)) {
      g2s_.emplace_back(spl, dist);
    }

    return;
  }

  if (spl->ends_special()) {
    initial_special_ends_.emplace_back(spl, dist);
  }

  auto const is_fwg1_length = spl->from_station_route() && spl->is_general() &&
                              !spl->length_.is_infinity();
  if (is_fwg1_length && spl->length_ >= -dist) {
    active_fwg1_length_.emplace_back(spl, dist);
    return;
  }

  auto const is_fwg1_infinite = spl->from_station_route() &&
                                spl->is_general() && spl->length_.is_infinity();
  auto const before_g1 = !wirkende_g1_.has_value();
  auto const no_set_or_same_dist =
      !fwg1_infinite_.has_value() ||
      (fwg1_infinite_->dist_ == dist && fwg1_infinite_->speed() > spl->limit_);

  if (is_fwg1_infinite && before_g1 && no_set_or_same_dist) {
    fwg1_infinite_.emplace(spl, dist);
    return;
  }
}

void speed_limit_manager::initialize_default(default_values const& defaults) {
  default_.type_ = speed_limit::type::general;
  default_.affects_ = speed_limit::affects::all;

  default_.poa_ = speed_limit::poa::here;
  default_.source_ = speed_limit::source::infrastructure;
  default_.limit_ = defaults.stationary_speed_limit_;
  default_.length_ = si::length::infinity();

  if (!wirkende_g1_.has_value()) {
    wirkende_g1_.emplace(&default_, si::length::min());
  }
}

void speed_limit_manager::initialize_from_infra(train const& train,
                                                infrastructure const& infra,
                                                si::length current_dist) {
  auto curr_node = train.first_station_route(infra)->nodes().front();

  auto prev_element = curr_node->element_;
  while (curr_node->reverse_ahead() != nullptr) {
    current_dist -= curr_node->element_->get_distance(prev_element);
    prev_element = curr_node->element_;

    if (curr_node->is(type::SPEED_LIMIT)) {
      auto spl = &infra->graph_.get_element_data<speed_limit>(curr_node);

      if (wirkende_g1_.has_value() && spl == wirkende_g1_->limit_) {
        // TODO(julian) this termination is just a quick fix
        // this loop should terminate when leaving at the FBN border
        break;
      }

      add_initial_speed_limit(train, spl, current_dist);
    }

    curr_node = curr_node->reverse_ahead();
  }
}

si::length speed_limit_manager::initialize_from_sr(
    train const& train, infrastructure const& infra) {
  utls::expect(!train.sequence_points_.empty(), "no sequence points in train");

  auto const& start = train.sequence_points_.front();

  // TODO(julian) determine from which start node we should start
  // when it is an transit as first sequence point
  utls::expect(start.is_halt(), "first sequence point is not halt");

  auto current_dist = si::length::zero();
  auto prev_element = start.get_node(infra)->element_;

  for (auto const& rn :
       train.first_station_route(infra)->from_bwd(start.idx_)) {
    current_dist -= rn.node_->element_->get_distance(prev_element);
    prev_element = rn.node_->element_;

    if (rn.omitted_) continue;

    if (rn.node_->is(type::SPEED_LIMIT)) {
      auto const spl = rn.get_speed_limit(infra);
      add_initial_speed_limit(train, spl, current_dist);
    }

    for (auto const& extra_spl : rn.extra_spls_) {
      add_initial_speed_limit(train, extra_spl, current_dist);
    }
  }

  return current_dist;
}

void speed_limit_manager::initialize(train const& train,
                                     infrastructure const& infra) {
  auto current_dist = si::length::zero();
  if (!train.break_in_) current_dist = initialize_from_sr(train, infra);
  initialize_from_infra(train, infra, current_dist);
  initialize_default(infra->defaults_);
}

// adding the new speed limit to the active speed limits
bool speed_limit_manager::add_here_general(active_speed_limit const& new_spl) {
  // if same mileage take the minimum, otherwise replace
  wirkende_g1_ =
      wirkende_g1_.has_value() && wirkende_g1_->dist_ == new_spl.dist_
          ? std::min(*wirkende_g1_, new_spl)
          : new_spl;

  fwg1_infinite_.reset();

  auto const before_new_spl = [&](auto&& s) {
    return s.original_dist_ <= new_spl.original_dist_;
  };

  if (utls::all_of(s1s_, before_new_spl)) s1s_.clear();

  if (!fws1s_.empty() && utls::all_of(fws1s_, before_new_spl))
    s1s_.clear(), fws1s_.clear();

  return true;
}

bool speed_limit_manager::add_here_special(active_speed_limit const& new_spl) {
  auto const same_specialties_as = [&](active_speed_limit const& spl) {
    return spl.limit_->same_specialties(*new_spl.limit_);
  };

  auto const before_new = [&](active_speed_limit const& s) {
    return s.limit_->same_specialties(*new_spl.limit_) &&
           s.original_dist_ <= new_spl.original_dist_;
  };

  auto const it = utls::find_if(g2s_, same_specialties_as);

  if (it != std::end(g2s_)) {
    // if same mileage take the minimum, otherwise replace
    *it = it->dist_ == new_spl.dist_ ? std::min(*it, new_spl) : new_spl;
  } else {
    g2s_.push_back(new_spl);
  }

  if (utls::all_of(s2s_, before_new)) utl::erase_if(s2s_, same_specialties_as);

  return true;
}

bool speed_limit_manager::add_station_route_infinite(
    active_speed_limit const& new_spl) {
  fwg1_infinite_ =
      fwg1_infinite_.has_value() && fwg1_infinite_->dist_ == new_spl.dist_
          ? std::min(*fwg1_infinite_, new_spl)
          : new_spl;

  auto const before_new = [&](auto&& s) {
    return s.original_dist_ <= new_spl.original_dist_;
  };

  if (utls::all_of(s1s_, before_new)) s1s_.clear();

  if (!fws1s_.empty() && utls::all_of(fws1s_, before_new))
    s1s_.clear(), fws1s_.clear();

  return true;
}

bool speed_limit_manager::add_station_route_length(
    active_speed_limit const& new_spl) {
  utls::expect(new_spl.dist_ == new_spl.original_dist_);
  utls::expect(new_spl.limit_->length_.is_valid(), "got invalid length");
  utls::expect(!new_spl.limit_->length_.is_infinity(), "got inf length");
  utls::expect(new_spl.limit_->length_ < si::length::max(), "got max length");

  auto const before_new = [&](auto&& s) {
    return s.original_dist_ <= new_spl.original_dist_;
  };

  if (utls::all_of(s1s_, before_new)) s1s_.clear();

  if (!fws1s_.empty() && utls::all_of(fws1s_, before_new))
    s1s_.clear(), fws1s_.clear();

  if (fwg1_infinite_.has_value() && fwg1_infinite_->dist_ != new_spl.dist_) {
    fwg1_infinite_.reset();
  }

  active_fwg1_length_.push_back(new_spl);

  return true;
}

std::optional<active_speed_limit> speed_limit_manager::get_wirkende_fwg1()
    const {
  std::optional<active_speed_limit> result;

  auto const min = utls::min_element(active_fwg1_length_);
  if (min != std::end(active_fwg1_length_)) {
    result = *min;
  }

  if (!fwg1_infinite_.has_value()) return result;

  result =
      result.has_value() ? std::min(*result, *fwg1_infinite_) : *fwg1_infinite_;

  return result;
}

bool speed_limit_manager::add_station_route_length_end(
    active_speed_limit const& new_spl) {
  utls::expect(new_spl.dist_ > new_spl.original_dist_);

  auto const it = utls::find_if(active_fwg1_length_, [&](auto&& spl) {
    return spl.limit_ == new_spl.limit_;
  });

  utls::expect(it != std::end(active_fwg1_length_));

  active_fwg1_length_.erase(it);

  return true;
}

bool speed_limit_manager::add_station_route_length_based(
    active_speed_limit const& new_spl) {
  if (new_spl.dist_ > new_spl.original_dist_) {
    return add_station_route_length_end(new_spl);
  } else {
    return add_station_route_length(new_spl);
  }
}

bool speed_limit_manager::add_last_signal_general(
    active_speed_limit const& new_spl) {
  auto const different_ms_as = [&](active_speed_limit const& spl) {
    return spl.dist_ != new_spl.dist_;
  };

  utl::erase_if(s1s_, different_ms_as);
  utl::erase_if(fws1s_, different_ms_as);
  utl::erase_if(s2s_, different_ms_as);

  s1s_.push_back(new_spl);

  // TODO(julian) conflicting documentation on this one ...
  // wirkende_g1_.reset();
  fwg1_infinite_.reset();

  return true;
}

bool speed_limit_manager::add_last_signal_special(
    active_speed_limit const& new_spl) {
  auto const same_specialties_as = [&](active_speed_limit const& spl) {
    return spl.limit_->same_specialties(*new_spl.limit_);
  };

  if (g2s_.empty() &&
      utls::find_if(s2s_, same_specialties_as) == std::end(s2s_)) {
    return false;
  }

  auto const different_ms_as = [&](active_speed_limit const& spl) {
    return spl.dist_ != new_spl.dist_;
  };

  utl::erase_if(s1s_, different_ms_as);
  utl::erase_if(fws1s_, different_ms_as);
  utl::erase_if(s2s_, different_ms_as);

  utl::erase_if(g2s_, same_specialties_as);

  s2s_.push_back(new_spl);

  return true;
}

bool speed_limit_manager::add_ends_special(active_speed_limit const& new_spl) {
  auto const same_specialties_as = [&](active_speed_limit const& spl) {
    return spl.limit_->same_specialties(*new_spl.limit_);
  };

  auto const old_g2s = g2s_.size();
  auto const old_s2s = s2s_.size();

  utl::erase_if(g2s_, same_specialties_as);
  utl::erase_if(s2s_, same_specialties_as);

  return old_g2s != g2s_.size() || old_s2s != s2s_.size();
}

// dispatching the new speed limit to the correct handler
bool speed_limit_manager::add_infrastructure(
    active_speed_limit const& new_spl) {
  auto const& spl = new_spl.limit_;

  if (spl->from_here() && spl->is_general()) {
    return add_here_general(new_spl);
  }

  if (spl->from_here() && spl->begins_special()) {
    return add_here_special(new_spl);
  }

  if (spl->from_last_signal() && spl->is_general()) {
    return add_last_signal_general(new_spl);
  }

  if (spl->from_last_signal() && spl->begins_special()) {
    return add_last_signal_special(new_spl);
  }

  if (spl->ends_special()) {
    return add_ends_special(new_spl);
  }

  throw utl::fail("did not manage to handle given speed limit");
}

bool speed_limit_manager::add_station_route(active_speed_limit const& new_spl) {
  utls::expect(new_spl.limit_->length_ != si::length::max());

  if (new_spl.limit_->from_last_signal() && new_spl.limit_->is_general()) {
    utls::sassert(new_spl.limit_->length_.is_infinity());
    return add_last_signal_general(new_spl);
  }

  if (new_spl.limit_->length_.is_infinity()) {
    return add_station_route_infinite(new_spl);
  }

  if (new_spl.limit_->length_.is_valid()) {
    return add_station_route_length_based(new_spl);
  }

  if (new_spl.limit_->ends_special()) {
    return add_ends_special(new_spl);
  }

  throw utl::fail("did not manage to handle given speed limit");
}

speed_limit_manager::speed_limit_manager(train const& train,
                                         infrastructure const& infra) {
  initialize(train, infra);
}

bool speed_limit_manager::add(active_speed_limit const& new_spl) {
  if (new_spl.limit_->from_infrastructure()) {
    return add_infrastructure(new_spl);
  } else if (new_spl.limit_->from_station_route()) {
    return add_station_route(new_spl);
  }

  throw utl::fail("did not manage to handle given speed limit");
}

// determine the current speed limit
si::speed speed_limit_manager::get_current_limit() const {
  auto curr = si::speed::max();

  if (!g2s_.empty() || !s2s_.empty()) {
    curr = !g2s_.empty() ? utls::min(g2s_)->speed() : curr;
    curr = !s2s_.empty() ? std::min(utls::min(s2s_)->speed(), curr) : curr;
    if (!s2s_.empty()) return curr;
  } else {
    curr = wirkende_g1_.has_value() ? wirkende_g1_->speed() : curr;
    auto const wirkende_fwg1 = get_wirkende_fwg1();
    curr = wirkende_fwg1.has_value() ? wirkende_fwg1->speed() : curr;
  }

  if (!fws1s_.empty()) {
    curr = std::min(utls::min_element(fws1s_)->speed(), curr);
    return curr;
  }

  if (!s1s_.empty()) {
    curr = std::min(utls::min_element(s1s_)->speed(), curr);
    return curr;
  }

  utls::sassert(curr.is_valid() && curr < si::speed::max(),
                "could not determine speed limit");
  return curr;
}

}  // namespace soro::runtime