#include "soro/infrastructure/interlocking/exclusion.h"

#include "utl/concat.h"
#include "utl/enumerate.h"
#include "utl/erase_duplicates.h"
#include "utl/erase_if.h"
#include "utl/logging.h"
#include "utl/pipes.h"
#include "utl/timer.h"
#include "utl/to_vec.h"

#include "cista/containers/bitvec.h"

#include "soro/utls/algo/overlap.h"
#include "soro/utls/coroutine/collect.h"
#include "soro/utls/coroutine/coro_map.h"

#include "soro/infrastructure/interlocking/exclusion2.h"

namespace soro::infra {

static const type_set POI_TYPES = {
    type::SIMPLE_SWITCH, type::CROSS,  type::MAIN_SIGNAL, type::HALT,
    type::BORDER,        type::BUMPER, type::TRACK_END};

struct exclusion_materials {
  explicit exclusion_materials(infrastructure const& infra)
      : element_used_by_{infra->graph_.elements_.size()},
        working_sets_{infra->graph_.elements_.size()},
        path_working_sets_{infra->interlocking_.routes_.size()} {}

  // size = #elements
  // every ir that uses the given POI element
  std::vector<std::vector<interlocking_route::id>> element_used_by_;

  // size = #elements
  // every POI element that is reachable from the given POI element
  std::vector<std::vector<element_id>> working_sets_;

  // size = #IRs
  // every POI element that is reacheable by a single IR
  std::vector<std::vector<element_id>> path_working_sets_;
};

exclusion_materials get_exclusion_materials(infrastructure const& infra) {
  utl::scoped_timer const t("Constructing exclusion materials used by IRs");

  exclusion_materials mats(infra);

  for (auto const& [ir_source_node, starting_at] :
       utl::enumerate(infra->interlocking_.starting_at_)) {

    if (starting_at.empty()) {
      continue;
    }

    for (auto const ir_id : starting_at) {

      auto const& ir = infra->interlocking_.routes_[ir_id];

      //       assert
      //      if (ir_source_node != ir.first_node(infra)->id_) {
      //        std::cout << "error\n";
      //      }

      for (auto const& rn : ir.iterate(infra)) {
        if (!POI_TYPES.contains(rn.node_->type())) {
          continue;
        }

        // TODO(julian) this only adds the elements in one direction,
        // missing the other direction for now
        mats.element_used_by_[rn.node_->element_->id()].emplace_back(ir.id_);
        mats.working_sets_[ir.first_node(infra)->element_->id()].emplace_back(
            rn.node_->element_->id());
        mats.path_working_sets_[ir.id_].emplace_back(rn.node_->element_->id());
      }
    }
  }

  for (auto& v : mats.element_used_by_) {
    utl::erase_duplicates(v);
  }

  for (auto& v : mats.working_sets_) {
    utl::erase_duplicates(v);
  }

  for (auto& v : mats.path_working_sets_) {
    utl::erase_duplicates(v);
  }

  //  auto expanded_working_sets = mats.working_sets_;
  //  for (auto const& working_set : mats.working_sets_) {
  //    for (auto const&kk)
  //
  //  }

  return mats;
}

void clean_up_working_sets(
    soro::vector<soro::vector<exclusion_set>>& working_sets) {

  soro::size_t counter = 0;
  for (auto& sets : working_sets) {
    if (sets.empty()) {
      continue;
    }

    ++counter;

    if (counter % 1000 == 0) {
      std::cout << counter << std::endl;
    }

    auto min_first = sets.front().first_;
    auto max_last = sets.front().last_;
    for (auto const& set : sets) {
      min_first = std::min(min_first, set.first_);
      max_last = std::max(max_last, set.last_);
    }

    for (auto& set : sets) {
      set.set_first(min_first);
      set.set_last(max_last);
    }

    for (auto i = 0U; i < sets.size(); ++i) {
      for (auto j = 0U; j < sets.size(); ++j) {
        if (sets[i].empty() || sets[j].empty() || i == j) {
          continue;
        }

        if (sets[i].contains(sets[j])) {
          sets[j].clear();
        }
      }
    }

    utl::erase_if(sets, [](auto&& s) { return s.empty(); });
  }
}

soro::vector<exclusion_set> get_interlocking_exclusion_sets(
    infrastructure const& infra) {

  //  auto const element_used_by = get_element_used_by(infra);

  auto const exclusion_mats = get_exclusion_materials(infra);

  auto sets = soro::to_vec(exclusion_mats.element_used_by_,
                           [](auto&& s) { return exclusion_set(s); });

  std::cout << "sets done" << std::endl;

  auto working_sets =
      soro::to_vec(exclusion_mats.working_sets_, [&](auto&& ws) {
        return soro::to_vec(ws, [&](auto&& e_id) { return sets[e_id]; });
      });

  std::cout << "working sets done" << std::endl;

  {
    std::size_t non_empty = 0;
    std::size_t size = 0;
    std::vector<std::size_t> sizes;
    for (auto const& set : working_sets) {
      if (set.empty()) {
        continue;
      }

      ++non_empty;
      size += set.size();
      sizes.emplace_back(set.size());
    }

    std::sort(std::begin(sizes), std::end(sizes));

    std::cout << "working sets before\n";
    std::cout << "non empty: " << non_empty << '\n';
    std::cout << "avg diff: " << size / non_empty << '\n';
    std::cout << "25%: " << sizes[sizes.size() * 0.25] << '\n';
    std::cout << "50%: " << sizes[sizes.size() * 0.5] << '\n';
    std::cout << "75%: " << sizes[sizes.size() * 0.75] << '\n';
    std::cout << "90%: " << sizes[sizes.size() * 0.9] << '\n';
    std::cout << "99%: " << sizes[sizes.size() * 0.99] << '\n';
  }

  clean_up_working_sets(working_sets);

  {
    std::size_t non_empty = 0;
    std::size_t size = 0;
    std::vector<std::size_t> sizes;
    for (auto const& set : working_sets) {
      if (set.empty()) {
        continue;
      }

      ++non_empty;
      size += set.size();
      sizes.emplace_back(set.size());
    }

    std::sort(std::begin(sizes), std::end(sizes));

    std::cout << "working sets after\n";
    std::cout << "non empty: " << non_empty << '\n';
    std::cout << "avg diff: " << size / non_empty << '\n';
    std::cout << "25%: " << sizes[sizes.size() * 0.25] << '\n';
    std::cout << "50%: " << sizes[sizes.size() * 0.5] << '\n';
    std::cout << "75%: " << sizes[sizes.size() * 0.75] << '\n';
    std::cout << "90%: " << sizes[sizes.size() * 0.9] << '\n';
    std::cout << "99%: " << sizes[sizes.size() * 0.99] << '\n';
  }

  {
    std::size_t non_empty = 0;
    std::size_t size = 0;
    std::vector<std::size_t> sizes;
    for (auto const& set : exclusion_mats.working_sets_) {
      if (set.empty()) {
        continue;
      }

      ++non_empty;
      size += set.size();
      sizes.emplace_back(set.size());
    }

    std::sort(std::begin(sizes), std::end(sizes));

    std::cout << "working sets\n";
    std::cout << "non empty: " << non_empty << '\n';
    std::cout << "avg diff: " << size / non_empty << '\n';
    std::cout << "25%: " << sizes[sizes.size() * 0.25] << '\n';
    std::cout << "50%: " << sizes[sizes.size() * 0.5] << '\n';
    std::cout << "75%: " << sizes[sizes.size() * 0.75] << '\n';
    std::cout << "90%: " << sizes[sizes.size() * 0.9] << '\n';
    std::cout << "99%: " << sizes[sizes.size() * 0.99] << '\n';
  }

  {
    std::size_t non_empty = 0;
    std::size_t size = 0;
    std::vector<std::size_t> sizes;
    for (auto const& set : exclusion_mats.path_working_sets_) {
      if (set.empty()) {
        continue;
      }

      ++non_empty;
      size += set.size();
      sizes.emplace_back(set.size());
    }

    std::sort(std::begin(sizes), std::end(sizes));

    std::cout << "path working sets\n";
    std::cout << "non empty: " << non_empty << '\n';
    std::cout << "avg diff: " << size / non_empty << '\n';
    std::cout << "25%: " << sizes[sizes.size() * 0.25] << '\n';
    std::cout << "50%: " << sizes[sizes.size() * 0.5] << '\n';
    std::cout << "75%: " << sizes[sizes.size() * 0.75] << '\n';
    std::cout << "90%: " << sizes[sizes.size() * 0.9] << '\n';
    std::cout << "99%: " << sizes[sizes.size() * 0.99] << '\n';
  }

  //  utl::erase_if(sets, [](auto&& s) { return s.empty(); });

  return sets;
}

}  // namespace soro::infra
