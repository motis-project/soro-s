#include "soro/infrastructure/interlocking/exclusion.h"

#include "utl/erase_duplicates.h"
#include "utl/erase_if.h"
#include "utl/pipes.h"
#include "utl/timer.h"

#include "soro/utls/algo/overlap.h"
#include "soro/utls/algo/remove_after.h"
#include "soro/utls/algo/remove_until.h"

#include "soro/infrastructure/base_infrastructure.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"

namespace soro::infra {

// std::vector<section::id> get_connecting_sections(element_ptr e1, element_ptr
// e2,
//                                                  graph const& net) {
//   auto const& e1_sections = net.element_id_to_section_ids_[e1->id()];
//   auto const& e2_sections = net.element_id_to_section_ids_[e2->id()];
//
//   return utl::all(e1_sections) | utl::remove_if([&](auto&& section_id) {
//            return !utls::contains(e2_sections, section_id);
//          }) |
//          utl::vec();
// }
//
// soro::vector<section::id> get_sections(interlocking_route const& ssr,
//                                        base_infrastructure const& iss) {
//   auto section_elements =
//       utl::all(ssr.nodes()) |
//       utl::transform([](auto&& node) { return node->element_; }) |
//       utl::remove_if([](auto&& e) { return e->is_track_element(); }) |
//       utl::vec();
//
//   soro::vector<section::id> section_ids;
//   if (ssr.starts_on_ms()) {
//     section_ids.push_back(
//         iss.graph_
//             .element_id_to_section_ids_[ssr.nodes().front()->element_->id()]
//             .front());
//   }
//
//   for (auto const [e1, e2] : utl::pairwise(section_elements)) {
//     auto const connecting_sections =
//         get_connecting_sections(e1, e2, iss.graph_);
//
//     utl::verify(!connecting_sections.empty(),
//                 "All section elements in a signal station route must be "
//                 "connected pairwise");
//
//     if (connecting_sections.size() == 1) {
//       section_ids.push_back(connecting_sections.front());
//       continue;
//     }
//
//     // handle the following case:
//     // two elements are connected more than once, so there exist more than
//     one
//     // connecting section. Appraoch: Just check which section contains a
//     track
//     // element from nodes_. This requires that there is at least one track
//     // element in the section
//     auto it = utls::find_if(connecting_sections, [&](auto&& section_id) {
//       auto const& section = iss.graph_.sections_[section_id];
//
//       utl::verify(section.elements_.size() > 2,
//                   "Need at least a single track element in this section, so "
//                   "the following approach works.");
//
//       auto const eles = ssr.elements();
//       return utls::overlap_non_sorted(std::cbegin(section.elements_) + 1,
//                                       std::cend(section.elements_) - 1,
//                                       std::cbegin(eles), std::cend(eles));
//     });
//
//     if (it != std::cend(connecting_sections)) {
//       section_ids.push_back(*it);
//       continue;
//     }
//
//     // Could not find a suitable section from the set of connecting sections.
//     // Conclusion: The section we use contains no element we care about
//     // this means it only contains track elements facing the opposite
//     direction it = utls::find_if(connecting_sections, [&, e = e1](auto&&
//     section_id) {
//       auto const& section = iss.graph_.sections_[section_id];
//       bool const rising = section.elements_.front()->id() == e->id();
//
//       return std::all_of(std::cbegin(section.elements_) + 1,
//                          std::cend(section.elements_) - 1,
//                          [&](auto&& s_e) { return s_e->rising() != rising;
//                          });
//     });
//
//     utl::verify(it != std::cend(connecting_sections),
//                 "Could not find a suitable connection in connecting
//                 sections");
//
//     section_ids.push_back(*it);
//   }
//
//   if (ssr.ends_on_ms() || ssr.nodes().back()->is(type::HALT)) {
//     section_ids.push_back(
//         iss.graph_
//             .element_id_to_section_ids_[ssr.nodes().back()->element_->id()]
//             .front());
//   }
//
//   if (section_ids.size() == 2 && section_ids.front() == section_ids.back()) {
//     section_ids.erase(std::end(section_ids) - 1);
//   }
//
//   return section_ids;
// }
//
// soro::vector<element_ptr> get_exclusion_elements(
//     interlocking_route const& ssr, base_infrastructure const& iss) {
//   soro::vector<element_ptr> elements;
//
//   auto section_ids = get_sections(ssr, iss);
//
//   // gather every section element used by the signal station route
//   auto section_elements =
//       utl::all(ssr.nodes()) |
//       utl::transform([](auto&& node) { return node->element_; }) |
//       utl::remove_if(
//           [](auto&& element) { return element->is_track_element(); }) |
//       utl::unique() | utl::vec();
//
//   auto const handle_first_section = [&]() {
//     auto const& first_sec = iss.graph_.sections_[section_ids.front()];
//     auto const& first_sec_element = section_elements.front();
//
//     utls::append(elements, first_sec.elements_);
//
//     bool const reverse =
//         ssr.starts_on_ms()
//             ? first_sec.elements_.front()->id() == first_sec_element->id()
//             : first_sec.elements_.front()->id() != first_sec_element->id();
//
//     if (reverse) {
//       std::reverse(std::begin(elements), std::end(elements));
//     }
//   };
//
//   // handle [2, ..., n - 1] sections
//   auto const handle_sections = [&]() {
//     auto from_it = std::cbegin(section_ids) + 1;
//     auto to_it = std::cend(section_ids) - 1;
//
//     auto curr_sec_element_idx = ssr.starts_on_ms() ? 1UL : 2UL;
//     for (auto const& section_id : utls::make_range(from_it, to_it)) {
//       auto const& section = iss.graph_.sections_[section_id];
//       auto const& section_element = section_elements[curr_sec_element_idx];
//
//       utls::append(elements, section.elements_);
//
//       bool const reverse =
//           section.elements_.front()->id() == section_element->id();
//
//       if (reverse) {
//         std::reverse(std::end(elements) -
//                          static_cast<ptrdiff_t>(section.elements_.size()),
//                      std::end(elements));
//       }
//
//       ++curr_sec_element_idx;
//     }
//   };
//
//   auto const handle_last_section = [&]() {
//     auto const& last_sec = iss.graph_.sections_[section_ids.back()];
//     auto const last_sec_element = section_elements.back();
//
//     utls::append(elements, last_sec.elements_);
//
//     auto const reverse =
//         ssr.ends_on_ms()
//             ? last_sec.elements_.front()->id() != last_sec_element->id()
//             : last_sec.elements_.front()->id() == last_sec_element->id();
//
//     if (reverse) {
//       std::reverse(std::end(elements) -
//                        static_cast<ptrdiff_t>(last_sec.elements_.size()),
//                    std::end(elements));
//     }
//   };
//
//   auto const handle_single_section_ssr = [&]() {
//     auto const& section = iss.graph_.sections_[section_ids.front()];
//
//     utls::append(elements, section.elements_);
//
//     auto start = utls::find(section.elements_,
//     ssr.nodes().front()->element_); auto end = utls::find(section.elements_,
//     ssr.nodes().back()->element_);
//
//     if (std::distance(end, start) > 0) {
//       std::reverse(std::begin(elements), std::end(elements));
//     }
//   };
//
//   if (section_ids.size() == 1) {
//     handle_single_section_ssr();
//   }
//
//   if (section_ids.size() > 1) {
//     handle_first_section();
//   }
//
//   if (section_ids.size() > 2) {
//     handle_sections();
//   }
//
//   if (section_ids.size() > 1) {
//     handle_last_section();
//   }
//
//   // remove the exclusion elements until the original first element
//   utls::remove_until(elements, [&](auto&& element) {
//     return element->id() == ssr.nodes().front()->element_->id();
//   });
//
//   // remove the exclusion elements after the original last element
//   utls::remove_after(elements, [&](auto&& element) {
//     return element->id() == ssr.nodes().back()->element_->id();
//   });
//
//   utls::unique_erase(elements);
//   return elements;
// }
//
///*
// * Returns for a given signal station route all potential conflicting signal
// * station routes.
// *
// * For now it does the following: Gather every station that is touched by
// * the given signal station route. Then, gather every signal station route
// * touching the gathered stations. Afterwards erase all duplicates.
// *
// */
// auto get_conflict_candidates(ir_ptr ssr, interlocking_subsystem const& ssr_m)
// {
//  soro::vector<ir_ptr> candidates;
//
//  utl::all(ssr->station_routes_) |
//      utl::transform([](auto&& sr) { return sr->station_; }) |  // NOLINT
//      utl::unique() |  // NOLINT
//      utl::transform(
//          [&](auto&& station) { return ssr_m.station_to_irs_[station->id_]; })
//          |
//      utl::for_each([&](auto&& ssr_vec) { utls::append(candidates, ssr_vec);
//      });
//
//  utls::sort(candidates);
//  utls::unique_erase(candidates);
//
//  return candidates;
//}

struct exclusion_matrix {

  void set(std::size_t const idx1, std::size_t const idx2, bool const val) {
    auto const min = std::min(idx1, idx2);
    auto const max = std::max(idx1, idx2);
    bits_[min][max - min] = val;
  }

  bool test(std::size_t const idx1, std::size_t const idx2) const {
    auto const min = std::min(idx1, idx2);
    auto const max = std::max(idx1, idx2);
    return bits_[min][max - min];
  }

  soro::vector<std::vector<bool>> bits_;
};

exclusion_matrix make_exclusion_matrix(std::size_t const entries) {
  exclusion_matrix em;

  em.bits_.resize(entries);
  for (auto i = 0UL; i < entries; ++i) {
    em.bits_[i].resize(entries - i, false);
    //    em.bits_[i] = std::vector<bool>(entries - i, false);
  }

  return em;
}

soro::vector<soro::vector<ir_ptr>> get_ssr_conflicts(
    base_infrastructure const&, interlocking_subsystem const&) {
  utl::scoped_timer const conflicts_timer(
      "Calculating signal station route conflicts");
  return {};
  //  auto const sorted_exclusion_elements =
  //      soro::to_vec(ssr_m.interlocking_routes_, [&](auto&& ssr) {
  //        auto elements = get_exclusion_elements(*ssr, iss);
  //        utls::sort(elements);
  //        return elements;
  //      });
  //
  //  auto const conflicts = [&](ir_ptr ssr1, ir_ptr ssr2) {
  //    // a ssr is in conflict with itself
  //    if (ssr1->id_ == ssr2->id_) {
  //      return true;
  //    }
  //
  //    // two ssrs which follow each other are not in conflict, although they
  //    // share a single element: the last/first one (the main signal).
  //    if (ssr1->follows(ssr2) || ssr2->follows(ssr1)) {
  //      return false;
  //    }
  //
  //    return utls::overlap(sorted_exclusion_elements[ssr1->id_],
  //                         sorted_exclusion_elements[ssr2->id_]);
  //  };
  //
  //  return soro::to_vec(ssr_m.interlocking_routes_, [&](auto&& ssr) {
  //    soro::vector<ir_ptr> result = get_conflict_candidates(ssr, ssr_m);
  //
  //    utl::erase_if(result,
  //                  [&](auto&& candidate) { return !conflicts(ssr, candidate);
  //                  });
  //
  //    return result;
  //  });
}

}  // namespace soro::infra
