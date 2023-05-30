#pragma once

#include <limits>
#include "soro/infrastructure/infrastructure.h"
#include "soro/simulation/ordering/ordering_graph.h"
#include "soro/timetable/sequence_point.h"
#include "soro/timetable/timetable.h"

namespace soro::simulation {

struct simulation_graph {
  using dependency_key = uint32_t;

  using timetable_dependency = tt::sequence_point::optional_ptr;
  using timetable_dependencies_t = soro::vector<timetable_dependency>;

  using train_dependency = uint32_t;
  using train_dependencies_t = soro::vecvec<dependency_key, train_dependency>;

  struct node {
    using id = uint32_t;

    id get_id(simulation_graph const& sg) const;

    //    bool has_timetable_dependency(simulation_graph const& sg) const;
    //    bool has_train_dependencies(simulation_graph const& sg) const;
    //
    //    timetable_dependency timetable_dependency(simulation_graph const& sg)
    //    const; train_dependencies_t::const_bucket train_dependencies(
    //        simulation_graph const& sg) const;

    infra::element_id element_id_{infra::element::INVALID};
  };

  struct range {
    auto iterate(simulation_graph const& sg) const {
      return utls::it_range(std::begin(sg.nodes_) + from_,
                            std::begin(sg.nodes_) + to_);
    }

    soro::size_t from_;
    soro::size_t to_;
  };

  struct interlocking_group : range {
    using offset = uint16_t;

    static constexpr offset INVALID_OFFSET = std::numeric_limits<offset>::max();

    interlocking_group(soro::size_t const from, soro::size_t const to,
                       soro::optional<offset> const approach_signal,
                       soro::optional<offset> const halt)
        : range(from, to), approach_signal_{approach_signal}, halt_{halt} {}

    auto skip_entry_ms(simulation_graph const& sg) const {
      auto const skip_first = (halt_.has_value() && *halt_ == 0) ? 0 : 1;
      return utls::it_range(std::begin(sg.nodes_) + from_ + skip_first,
                            std::begin(sg.nodes_) + to_);
    }

    bool has_halt() const { return halt_.has_value(); }
    bool has_approach() const { return approach_signal_.has_value(); }

    bool approach_before_halt() const {
      utls::expect(has_halt(),
                   "approach_before_halt called on group without halt");
      utls::expect(has_approach(),
                   "approach_before_halt called on group without approach");

      return *halt_ > *approach_signal_;
    }

    soro::optional<offset> approach_signal_;
    soro::optional<offset> halt_;
  };

  simulation_graph() = default;
  simulation_graph(infra::infrastructure const& infra,
                   tt::timetable const& timetable, ordering_graph const& og);

  // nodes layout for n trips:
  // [ trip 1 nodes , trip 2 nodes ... trip n nodes]
  // trip i nodes layout for m interlocking routes in trip i:
  // [ ir 1 nodes , ir 2 nodes ... ir m nodes]
  soro::vector<node> nodes_;

  timetable_dependencies_t timetable_dependencies_;
  train_dependencies_t train_dependencies_;

  // ordering graph node id indexed
  soro::vector<interlocking_group> interlocking_groups_;

  // span into nodes_, indexed with train id
  soro::vector<range> trips_;

  soro::vector<ordering_node::id> simulation_node_to_ordering_node_;
};

}  // namespace soro::simulation
