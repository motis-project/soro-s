#include "soro/simulation/route_ordering.h"

#include "utl/enumerate.h"

#include "soro/runtime/runtime.h"
#include "soro/utls/std_wrapper/std_wrapper.h"

namespace soro::simulation {

using namespace soro::tt;
using namespace soro::infra;

route_ordering get_route_ordering(infra::infrastructure const& infra,
                                  timetable const&) {
  route_ordering ordering(infra->interlocking_.interlocking_routes_.size());

  //  for (auto const& tr : tt->trains_) {
  //    auto const stamps = runtime_calculation(*tr, *infra,
  //    {type::MAIN_SIGNAL});
  //
  //    utl::verify(tr->path_.size() == stamps.times_.size() + 1,
  //                "Differing amounts of signal station routes in train path
  //                and " "main signals in running time calculation
  //                timestamps");
  //
  //    for (auto const [idx, ssr] : utl::enumerate(tr->path_)) {
  //      auto const from =
  //          idx == 0 ? tr->first_departure() : stamps.times_[idx -
  //          1].departure_;
  //      auto const to = idx == tr->path_.size() - 1 ? tr->last_arrival()
  //                                                  :
  //                                                  stamps.times_[idx].arrival_;
  //
  //      // the train tr will actually use the signal station route ssr
  //      //  -> insert a usage for it
  //      //      ordering[ssr->id_].push_back(
  //      //          {.from_ = from, .to_ = to, .train_id_ = tr->id_});
  //
  //      // the train tr will block signal station routes in conflict with ssr
  //      //  -> insert a usage for all conflicting routes
  //      for (auto const& conflict : infra->ssr_manager_.conflicts_[ssr->id_])
  //      {
  //        ordering[conflict->id_].push_back(
  //            {.from_ = from, .to_ = to, .train_id_ = tr->id_});
  //      }
  //    }
  //  }
  //
  //  for (auto& usage_order : ordering) {
  //    utls::sort(usage_order, [](auto&& usage1, auto&& usage2) {
  //      return usage1.from_ < usage2.from_;
  //    });
  //  }

  return ordering;
}

usage_index get_usage_index(route_ordering const& ordering,
                            train::id const train,
                            interlocking_route::id const ssr) {
  for (auto const [idx, order] : utl::enumerate(ordering[ssr])) {
    if (order.train_id_ == train) {
      return idx;
    }
  }

  throw utl::fail(
      "Could not find train with ID {} using signal station route with ID {}.",
      train, ssr);
}

}  // namespace soro::simulation
