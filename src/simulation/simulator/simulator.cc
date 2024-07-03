#include "soro/simulation/simulator/simulator.h"

#include <algorithm>
#include <ranges>

#include "soro/base/soro_types.h"
#include "soro/base/time.h"

#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/all_of.h"
#include "soro/utls/std_wrapper/count_if.h"

#include "soro/infrastructure/graph/node.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/infrastructure.h"

#include "soro/timetable/timetable.h"
#include "soro/timetable/train.h"

#include "soro/runtime/common/event.h"
#include "soro/runtime/common/signal_time.h"
#include "soro/runtime/runtime_calculator.h"

#include "soro/simulation/graph/graph.h"

namespace soro::sim {

using namespace soro::tt;
using namespace soro::infra;
using namespace soro::runtime;

simulator::simulator(infrastructure const& infra, timetable const& tt,
                     graph const& sg) {
  results_.resize(sg.nodes_.size());

  auto const& record_types = graph::occuring_types();

  auto const trips =
      sg.trips_ | std::views::transform([&](auto&& trip) -> auto const& {
        return static_cast<tt::train::trip const&>(trip);
      });

  calculator_ = runtime::calculator(trips, infra, tt, record_types);
}

signal_time get_signal_time(graph::simulation_group const& sg, graph const& g,
                            simulator::results_t const& results) {
  signal_time result;

  auto const first = sg.front(g);
  if (first.type_ == type::MAIN_SIGNAL) {
    result.main_ = first.element_id_;
  } else if (first.type_ == type::APPROACH_SIGNAL) {
    result.approach_ = first.element_id_;
    auto const next = first.get_interlocking_group(g).back(g);
    utls::sassert(next.type_ == type::MAIN_SIGNAL);
    result.main_ = next.element_id_;
  } else {
    utls::sassert(false, "first node must be a signal");
  }

  // set maximum of all dependencies as signal time
  for (auto const dep : sg.front(g).in(g)) {
    utls::sassert(results[dep].arrival_ != INVALID<absolute_time>);
    result.time_ = std::max(result.time_, results[dep].arrival_);
  }

  return result;
}

void simulator::operator()(graph::simulation_group const& sg, graph const& g,
                           timetable const& tt) {
  auto const to_be_calculated = result_span(sg);

  utls::sasserts([&] {
    utls::sassert(to_be_calculated.size() == sg.size(),
                  "tbc must have same size as simulation group has nodes");

    auto const all_dep_nodes = utls::count_if(sg.nodes(g), [&](auto&& node) {
      return node.has_train_dependencies(g);
    });

    auto const border_dep_nodes =
        static_cast<soro::size_t>(sg.front(g).has_train_dependencies(g)) +
        static_cast<soro::size_t>(sg.back(g).has_train_dependencies(g));

    utls::sassert(
        all_dep_nodes == border_dep_nodes,
        "either none, first or last, or both nodes can have dependencies");

    for (auto const& dep : sg.front(g).in(g)) {
      utls::sassert(results_[dep].identity_ == dep, "unfulfilled dependency");
    }

    utls::sassert(utls::all_of(sg.front(g).in(g),
                               [&](auto&& d) {
                                 return results_[d].arrival_ !=
                                        INVALID<absolute_time>;
                               }),
                  "unfillfilled dependency");
  });

  auto const st = get_signal_time(sg, g, results_);

  graph::simulation_group::offset idx = 0;
  auto const& trip = sg.get_trip(g);
  auto const event_cb = [&](runtime::event const& e) {
    utls::sassert(sg.at(g, idx).element_id_ == e.element_->get_id());

    to_be_calculated[idx].identity_ = sg.at(g, idx).get_id(g);
    to_be_calculated[idx].arrival_ = trip.anchor_ + e.arrival_;

    ++idx;
  };

  auto const terminate_cb = [&](node::ptr const n) {
    return n->element_->get_id() == sg.back(g).element_id_;
  };

  calculator_(sg.get_trip(g), tt, st, event_cb, terminate_cb);
}

}  // namespace soro::sim