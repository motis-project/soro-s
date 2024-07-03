#pragma once

#include "soro/utls/std_wrapper/any_of.h"

#include "soro/timetable/timetable.h"

#include "soro/runtime/common/get_intervals.h"
#include "soro/runtime/common/interval.h"
#include "soro/runtime/rk4_runtime.h"

namespace soro::runtime {

struct calculator {
  calculator() = default;

  template <typename TripIterable>
  calculator(TripIterable const& trips, infra::infrastructure const& infra,
             tt::timetable const& tt, infra::type_set const& record_types) {
    runtime_ctxs_.reserve(utls::narrow<soro::size_t>(trips.size()));
    for (auto const& trip : trips) {
      auto const& train = tt->trains_[trip.train_id_];
      runtime_ctxs_.emplace_back(train, infra, record_types);
    }
  }

  void operator()(tt::train::trip const& trip, tt::timetable const& tt,
                  signal_time const& signal_time, EventCB const& event_cb,
                  TerminateCb const& terminate_cb) {
    auto& ctx = runtime_ctxs_[trip.id_];

    auto const& train = tt->trains_[trip.train_id_];

    auto should_terminate = false;
    while (!should_terminate) {
      interval const i{&ctx.intervals_.p_[ctx.idx_],
                       &ctx.intervals_.p_[ctx.idx_ + 1]};
      rk4::calculate_interval(i, train, trip, ctx, use_surcharge::no,
                              signal_time, event_cb);

      ++ctx.idx_;

      should_terminate = ctx.idx_ >= ctx.intervals_.p_.size() - 1 ||
                         utls::any_of(i.records(), [&](auto&& e) {
                           return terminate_cb(e.node_);
                         });
    }
  }

  bool finished(tt::train::trip const& trip) const {
    utls::expect(trip.id_ < runtime_ctxs_.size(), "invalid trip id");

    return runtime_ctxs_[trip.id_].intervals_.p_.size() - 1 ==
           runtime_ctxs_[trip.id_].idx_;
  }

  struct runtime_ctx : rk4::runtime_state {
    runtime_ctx(tt::train const& train, infra::infrastructure const& infra,
                infra::type_set const& record_types)
        : intervals_{get_intervals(train, record_types, infra)} {
      state_.train_state_.speed_ = train.start_speed_;
    }

    soro::size_t idx_{0};
    intervals intervals_;
    rk4::runtime_state state_;
  };

  soro::vector_map<tt::train::trip::id, runtime_ctx> runtime_ctxs_;
};

}  // namespace soro::runtime