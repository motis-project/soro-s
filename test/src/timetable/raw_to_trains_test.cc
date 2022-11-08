#include "doctest/doctest.h"

// TODO(julian) check this
// void check_ssr_run(timetable const& tt) {
//   for (auto const& train : tt) {
//     std::vector<node_ptr> sr_nodes;
//     std::vector<node_ptr> ssr_nodes;
//
//     std::vector<node_ptr> sr_omitted;
//     std::vector<node_ptr> ssr_omitted;
//
//     std::vector<speed_limit> sr_extra_spls;
//     std::vector<speed_limit> ssr_extra_spls;
//
//     for (auto const& ssr : train->ssr_run_.path_) {
//       utls::append(ssr_nodes, ssr->nodes_);
//
//       for (auto const& omit_idx : ssr->omitted_nodes_) {
//         ssr_omitted.push_back(ssr->nodes_[omit_idx]);
//       }
//
//       for (auto const& extra_spl : ssr->extra_speed_limits_) {
//         ssr_extra_spls.push_back(extra_spl);
//       }
//     }
//
//     for (auto const& entry : train->entries_) {
//       for (auto const& sr : entry.station_routes_) {
//         utls::append(sr_nodes, sr->nodes_);
//
//         for (auto const& omit_idx : sr->omitted_nodes_) {
//           sr_omitted.push_back(sr->nodes_[omit_idx]);
//         }
//
//         for (auto const& extra_spl : sr->extra_speed_limits_) {
//           sr_extra_spls.push_back(extra_spl);
//         }
//       }
//     }
//
//     CHECK_MESSAGE(ssr_nodes.size() == sr_nodes.size(),
//                   "Signal station route nodes do not correspond to the ISS "
//                   "station route nodes");
//
//     for (auto [n1, n2] : utl::zip(sr_nodes, ssr_nodes)) {
//       CHECK(n1->id_ == n2->id_);
//     }
//
//     for (auto [om1, om2] : utl::zip(sr_omitted, ssr_omitted)) {
//       CHECK(om1->id_ == om2->id_);
//     }
//
//     for (auto [spl1, spl2] : utl::zip(sr_extra_spls, ssr_extra_spls)) {
//       CHECK(spl1 == spl2);
//     }
//
//     std::size_t ssr_halt_count = utls::count(train->ssr_run_.halts_, true);
//     std::size_t sr_halt_count = utls::count_if(
//         train->entries_, [](auto&& entry) { return entry.halt(); });
//
//     CHECK_MESSAGE(sr_halt_count == ssr_halt_count,
//                   "Signal station route run and ISS station route run must "
//                   "have same amount of halts");
//   }
// }
