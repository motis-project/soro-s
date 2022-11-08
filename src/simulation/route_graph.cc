#include "soro/simulation/route_graph.h"

#include "cista/containers/hash_set.h"
#include "cista/containers/variant.h"
#include "utl/logging.h"
#include "utl/pairwise.h"
#include "utl/timer.h"

#include "soro/runtime/runtime.h"
#include "soro/simulation/route_ordering.h"
#include "soro/utls/graph/traversal.h"
#include "soro/utls/parallel_for.h"

namespace soro::simulation {

using namespace tt;
using namespace infra;
using namespace runtime;
using namespace soro::utls::literals;

// sim_graph::sim_graph(infra::infrastructure const& infra,
//                      tt::timetable const& tt) {
//
//   // --- Create the simulation nodes --- //
//   train_to_sim_nodes_.resize(tt->trains_.size());
//   ssr_to_node_.resize(tt->trains_.size());
//
//   for (auto const& tr : tt->trains_) {
//     sim_node_id const first_node = nodes_.size();
//
//     sim_node init_sn;
//     init_sn.id_ = nodes_.size();
//     init_sn.train_id_ = tr->id_;
//     init_sn.ssr_id_ = interlocking_route::INVALID;
//     init_sn.exit_dpd_ = get_init_dpd(tr->first_departure());
//     nodes_.push_back(init_sn);
//
//     for (auto const& ssr : tr->path_) {
//       sim_node sn;
//       sn.id_ = nodes_.size();
//       sn.train_id_ = tr->id_;
//       sn.ssr_id_ = ssr->id_;
//
//       nodes_.push_back(sn);
//
//       ssr_to_node_[tr->id_][ssr->id_] = sn.id_;
//     }
//
//     // create two sentinel node at the end of the simulation train run
//     nodes_.push_back({.id_ = nodes_.size(),
//                       .train_id_ = tr->id_,
//                       .ssr_id_ = interlocking_route::INVALID,
//                       .entry_dpd_ = {},
//                       .exit_dpd_ = {},
//                       .eotd_dpd_ = {}});
//
//     nodes_.push_back({.id_ = nodes_.size(),
//                       .train_id_ = tr->id_,
//                       .ssr_id_ = interlocking_route::INVALID,
//                       .entry_dpd_ = {},
//                       .exit_dpd_ = {},
//                       .eotd_dpd_ = {}});
//
//     train_to_sim_nodes_[tr->id_] = {first_node, nodes_.size()};
//   }
//
//   // --- Create simulation graph edges --- //
//
//   // first all 'trivial' edges; connect nodes belonging to a single train run
//   for (auto& sn : nodes_) {
//     if (sn.id_ != train_to_sim_nodes_[sn.train_id_].first) {
//       sn.train_predecessor_ = sn.id_ - 1;
//     }
//
//     if (sn.id_ != train_to_sim_nodes_[sn.train_id_].second - 1) {
//       sn.train_successor_ = sn.id_ + 1;
//     }
//   }
//
//   // these are the ordering edges; these impose an order in which trains
//   // use shared/contested resources (signal station routes)
//   auto const ordering = get_route_ordering(infra, tt);
//
//   for (auto const& train_run : tt) {
//     for (auto const& ssr : train_run->path_) {
//       for (auto const [first, second] : utl::pairwise(ordering[ssr->id_])) {
//         auto first_sim_node_id = ssr_to_node_[first.train_id_].at(ssr->id_);
//         auto second_sim_node_id =
//         ssr_to_node_[second.train_id_].at(ssr->id_);
//
//         // create the actual order edge
//         auto const from = first_sim_node_id + 2;
//         auto const to = second_sim_node_id;
//
//         nodes_[from].out_.push_back(to);
//         nodes_[to].in_.push_back(from);
//       }
//     }
//   }
//
//   // TODO(julian) instead of erasing, do not add them in the first place
//   for (auto& node : nodes_) {
//     utls::unique_erase(node.in_);
//     utls::unique_erase(node.out_);
//   }
//
//   //  utl::verify(!cycle_detection(*this), "Found cycle in sim_graph!");
// }
//
// void sim_graph::simulate(infra::infrastructure const& infra, tt::timetable&
// tt,
//                          bool const use_dists, std::size_t, std::size_t) {
//   for (auto& n : nodes_) {
//     if (!n.has_pred()) {
//       continue;
//     }
//
//     n.entry_dpd_ = {};
//     n.eotd_dpd_ = {};
//     n.exit_dpd_ = {};
//   }
//
//   propagate2(this, infra, tt, use_dists);
// }
//
// TimeDPD fold_max(TimeDPD const& dpd1, TimeDPD const& dpd2) {
//   TimeDPD result;
//
//   for (auto const [t1, p1] : dpd1) {
//     for (auto const [t2, p2] : dpd2) {
//       result.insert(std::max(t1, t2), p1 * p2);
//     }
//   }
//
//   return result;
// }
//
// TimeDPD get_eotd_dpd(sim_graph const& sg, sim_node_id const& sn_id) {
//   auto const& sn = sg.nodes_[sn_id];
//
//   if (sn.in_.empty()) {
//     TimeDPD default_dpd;
//     default_dpd.insert(utls::EPOCH, 1);
//     return default_dpd;
//   }
//
//   if (sn.in_.size() == 1) {
//     return sg.nodes_[sn.in_.front()].eotd_dpd_;
//   }
//
//   TimeDPD result = sg.nodes_[sn.in_.front()].eotd_dpd_;
//   for (std::size_t i = 1; i < sn.in_.size(); ++i) {
//     result = fold_max(result, sg.nodes_[sn.in_[i]].eotd_dpd_);
//   }
//
//   return result;
// }
//
// discrete_scenario calcuate_discrete_scenario(
//     infra::train_path const&, infra::infrastructure const&,
//     infra::interlocking_route const&, kilometer_per_hour const&,
//     utls::unixtime const&, utls::unixtime const&, utls::duration const&) {
//
//   //  auto const scenario_result = runtime_calculation_ssr(
//   //      tr, *infra, ssr.id_, start_time, si::from_km_h(init_speed.km_h_),
//   //      go_time, extra_stand_time);
//   discrete_scenario scenario_result;
//
//   return scenario_result;
//
//   //  auto const end_time = scenario_result.end_arrival_;
//   //  auto const eotd_arrival = scenario_result.eotd_arrival_;
//   //  auto const end_speed =
//   //      kilometer_per_hour{static_cast<kilometer_per_hour::member_t>(
//   //          si::as_km_h(scenario_result.end_speed_))};
//   //
//   //
//   //  return {eotd_arrival, end_time, end_speed};
// }
//
// kilometer_per_hour si_to_sim_kmh(si::speed const& s) {
//   return kilometer_per_hour{
//       static_cast<kilometer_per_hour::member_t>(si::as_km_h(s))};
// }
//
// void sim_node::compute_dists(sim_graph const& sg,
//                              soro::infra::infrastructure const& infra,
//                              soro::tt::timetable const& tt,
//                              bool const use_dists) {
//   std::cout << "Computing Dists for Node: " << this->id_ << '\n';
//
//   this->entry_dpd_ = sg.nodes_[this->train_predecessor()].exit_dpd_;
//
//   // check if last node of a train
//   if (!interlocking_route::valid(this->ssr_id_)) {
//     this->exit_dpd_ = this->entry_dpd_;
//
//     for (auto const& [time, speed_dpd] : this->exit_dpd_) {
//       auto const prob = utls::sum(speed_dpd.dpd_, probability_t(0.0));
//       this->eotd_dpd_.insert(time + 5_m, prob);
//     }
//
//     return;
//   }
//
//   auto const& ssr =
//   *infra->interlocking_.interlocking_routes_[this->ssr_id_]; auto const
//   folded_eotd_dpd = get_eotd_dpd(sg, id_);
//
//   for (auto const& [time, speed_dpd] : this->entry_dpd_) {
//     for (auto const [speed, prob] : speed_dpd) {
//       for (auto const [go_time, eotd_prob] : folded_eotd_dpd) {
//
//         if (use_dists && ssr.id_ == 409) {
//           auto const scenario_result =
//               calcuate_discrete_scenario((*tt)[train_id_], infra, ssr, speed,
//                                          time, go_time, utls::ZERO_DURATION);
//
//           if (scenario_result.eotd_arrival_ != utls::INVALID_TIME) {
//             this->eotd_dpd_.insert(scenario_result.eotd_arrival_,
//                                    prob * eotd_prob * 0.5f);
//           }
//
//           this->exit_dpd_.insert(scenario_result.end_arrival_,
//                                  si_to_sim_kmh(scenario_result.end_speed_),
//                                  prob * eotd_prob * 0.5f);
//
//           auto const scenario_result2 = calcuate_discrete_scenario(
//               (*tt)[train_id_], infra, ssr, speed, time, go_time, 6_s);
//
//           if (scenario_result2.eotd_arrival_ != utls::INVALID_TIME) {
//             this->eotd_dpd_.insert(scenario_result2.eotd_arrival_,
//                                    prob * eotd_prob * 0.5f);
//           }
//
//           this->exit_dpd_.insert(scenario_result2.end_arrival_,
//                                  si_to_sim_kmh(scenario_result2.end_speed_),
//                                  prob * eotd_prob * 0.5f);
//         } else {
//           auto const scenario_result =
//               calcuate_discrete_scenario((*tt)[train_id_], infra, ssr, speed,
//                                          time, go_time, utls::ZERO_DURATION);
//
//           if (scenario_result.eotd_arrival_ != utls::INVALID_TIME) {
//             this->eotd_dpd_.insert(scenario_result.eotd_arrival_,
//                                    prob * eotd_prob);
//           }
//
//           this->exit_dpd_.insert(scenario_result.end_arrival_,
//                                  si_to_sim_kmh(scenario_result.end_speed_),
//                                  prob * eotd_prob);
//         }
//       }
//     }
//   }
// }
//
// bool sim_node::is_sentinel() const {
//   return infra::interlocking_route::valid(ssr_id_);
// }
//
// bool sim_node::finished() const { return !exit_dpd_.empty(); }
//
// bool sim_node::ready(sim_graph const& sg) const {
//   return utls::all_of(this->in(),
//                       [&](auto&& in) { return sg.nodes_[in].finished(); });
// }
//
// bool cycle_detection(simulation::sim_graph const& sg) {
//   std::vector<bool> visited(sg.nodes_.size());
//   std::vector<bool> finished(sg.nodes_.size());
//
//   auto const cycle_detection_inner = [&](sim_node_id const n_outer) {
//     auto const cycle_detection_impl = [&](sim_node_id const n,
//                                           auto&& impl) -> bool {
//       if (finished[n]) {
//         return false;
//       }
//
//       if (visited[n]) {
//         return true;
//       }
//
//       visited[n] = true;
//
//       for (auto const& neighbour : sg.nodes_[n].out()) {
//         if (impl(neighbour, impl)) {
//           return true;
//         }
//       }
//
//       finished[n] = true;
//
//       return false;
//     };
//
//     return cycle_detection_impl(n_outer, cycle_detection_impl);
//   };
//
//   return utls::any_of(sg.train_to_sim_nodes_, [&](auto&& pair) {
//     return cycle_detection_inner(pair.first);
//   });
// }

}  // namespace soro::simulation
