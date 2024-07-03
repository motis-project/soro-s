#include "soro/runtime/common/interval.h"

#include <cassert>
#include <cstdint>
#include <algorithm>

#include "soro/base/soro_types.h"
#include "soro/base/time.h"

#include "soro/utls/sassert.h"

#include "soro/si/units.h"

#include "soro/infrastructure/brake_path.h"
#include "soro/infrastructure/graph/type.h"

#include "soro/rolling_stock/train_physics.h"

#include "soro/timetable/sequence_point.h"

namespace soro::runtime {

using namespace soro::rs;
using namespace soro::tt;
using namespace soro::infra;

soro::vector<record> const& interval::records() const { return p1_->records_; }

si::length interval::start_distance() const { return p1_->distance_; }

si::length interval::end_distance() const { return p2_->distance_; }

si::length interval::length() const {
  return end_distance() - start_distance();
}

bool interval::starts_on_stop() const {
  return p1_->sequence_point_.has_value() && (*p1_->sequence_point_)->is_stop();
}

bool interval::ends_on_stop() const {
  return p2_->sequence_point_.has_value() && (*p2_->sequence_point_)->is_stop();
}

bool interval::starts_on_signal() const { return p1_->has_signal(); }

bool interval::starts_on_signal(infra::type const t) const {
  return p1_->has_signal(t);
}

bool interval::ends_on_signal() const { return p2_->has_signal(); }

bool interval::ends_on_signal(infra::type const t) const {
  return p2_->has_signal(t);
}

signal const& interval::start_signal() const {
  utls::sassert(starts_on_signal(),
                "asking for start signal on non-signal interval");
  assert(p1_->last_signal_.has_value());

  return *p1_->last_signal_;
}

signal const& interval::end_signal() const {
  utls::sassert(ends_on_signal(),
                "asking for end signal on non-signal interval");
  assert(p1_->next_signal_.has_value());

  return *p1_->next_signal_;
}

signal const& interval::next_signal() const {
  utls::sassert(p1_->next_signal_.has_value(),
                "asking for next signal on non-signal interval");
  assert(p1_->next_signal_.has_value());

  return *p1_->next_signal_;
}

soro::optional<relative_time> interval::end_departure() const {
  utls::sassert(ends_on_stop(),
                "asking for end departure on non-halt interval");

  return (*p2_->sequence_point_)->departure_;
}

duration interval::min_stop_time() const {
  utls::sassert(ends_on_stop(),
                "asking for min stop time on non-halt interval");

  return (*p2_->sequence_point_)->min_stop_time();
}

si::speed interval::infra_limit() const { return p1_->limit_; }

si::speed interval::bwp_limit() const { return p1_->bwp_limit_; }

si::speed interval::speed_limit() const {
  return std::min(infra_limit(), bwp_limit());
}

si::speed interval::speed_limit(train_physics const& tp) const {
  return tp.max_speed(speed_limit());
}

si::speed interval::signal_limit() const { return p1_->signal_limit_; }

brake_path interval::brake_path() const { return p1_->brake_path_; }

si::length interval::brake_path_length() const {
  return p1_->brake_path_length_;
}

si::slope interval::slope() const { return p1_->slope_; }

si::slope interval::avg_slope() const { return p1_->avg_slope_; }

si::speed interval::target_speed() const {
  return ends_on_stop() ? si::speed::zero()
                        : std::min(p2_->limit_, p2_->bwp_limit_);
}

si::speed interval::target_speed(train_physics const& tp) const {
  return std::min(target_speed(), tp.max_speed(speed_limit()));
}

si::speed interval::target_signal_speed() const { return p2_->signal_limit_; }

sequence_point::optional_ptr interval::sequence_point() const {
  return p2_->sequence_point_;
}

interval& interval::operator++() {
  ++p1_;
  ++p2_;
  return *this;
}

interval& interval::operator--() {
  --p1_;
  --p2_;
  return *this;
}

interval interval::operator+(uint32_t const n) const {
  return {p1_ + n, p2_ + n};
}

interval interval::operator-(uint32_t const n) const {
  return {p1_ - n, p2_ - n};
}

bool interval::operator==(interval const& o) const {
  return o.p1_ == p1_ && o.p2_ == p2_;
}

interval const& interval::operator*() const { return *this; }

interval intervals::begin() const { return {p_.data(), p_.data() + 1}; }

interval intervals::end() const {
  return {p_.data() + p_.size() - 1, p_.data() + p_.size()};
}

interval intervals::rbegin() const {
  return {p_.data() + p_.size() - 2, p_.data() + p_.size() - 1};
}

interval intervals::rend() const { return {p_.data() - 1, p_.data()}; }

soro::vector<record> const& intervals::last_records() const {
  utls::expect(!p_.empty(), "no interval points in intervals?");
  return p_.back().records_;
}

}  // namespace soro::runtime
