#include "soro/infrastructure/exclusion/get_exclusion_sets.h"

#include <thread>

#include "range/v3/action/sort.hpp"
#include "range/v3/action/unique.hpp"
#include "range/v3/to_container.hpp"
#include "range/v3/view/join.hpp"
#include "range/v3/view/transform.hpp"
#include "range/v3/view/zip.hpp"

#include "utl/enumerate.h"
#include "utl/erase.h"
#include "utl/erase_duplicates.h"
#include "utl/erase_if.h"
#include "utl/parallel_for.h"
#include "utl/timer.h"

namespace soro::infra {

struct working_set {
  auto size() const { return exclusion_sets_.size(); }
  auto empty() const { return exclusion_sets_.empty(); }

  // the element that induces the working set
  element_id stems_from_{INVALID_ELEMENT_ID};
  soro::vector<exclusion_set::id> exclusion_sets_;
};

soro::vector<interlocking_route::ids> get_open_element_used_by(
    soro::vector<interlocking_route::ids> const& closed_element_used_by,
    infrastructure const& infra) {
  utl::scoped_timer const timer("creating open element used by");

  auto open_element_used_by = closed_element_used_by;

  for (auto const& ir : infra->interlocking_.routes_) {
    auto const first = ir.first_node(infra)->element_->id();
    utl::erase(open_element_used_by[first], ir.id_);

    auto const last = ir.last_node(infra)->element_->id();
    utl::erase(open_element_used_by[last], ir.id_);
  }

  return open_element_used_by;
}

struct element_exclusion_sets {
  exclusion_set::optional_id closed_;
  exclusion_set::optional_id starting_;
  exclusion_set::optional_id ending_;
};

struct exclusion_sets {
  soro::vector<element_exclusion_sets> element_to_exclusion_set_;
  soro::vector<exclusion_set> sets_;
};

exclusion_sets get_initial_exclusion_sets(
    soro::vector<interlocking_route::ids> const& closed_element_used_by,
    soro::vector<interlocking_route::ids> const& starting_at,
    soro::vector<interlocking_route::ids> const& ending_at,
    infrastructure const& infra) {
  utl::scoped_timer const timer("constructing initial exclusion sets");

  auto const element_count = closed_element_used_by.size();

  exclusion_sets result;
  // initial layout: every element gets at most 3 exclusion sets
  // [e0 closed, e0 starting, e0 ending, e1 closed, e1 starting, e1 ending, ...]
  result.sets_.resize(element_count * 3);

  result.element_to_exclusion_set_.resize(element_count);

  for (auto job_id = 0U; job_id < element_count; ++job_id) {
    auto const e_id = static_cast<infra::element_id>(job_id);
    auto const& ir_ids = closed_element_used_by[e_id];

    if (ir_ids.empty()) {
      continue;
    }

    auto& closed = result.sets_[static_cast<exclusion_set::id>(e_id * 3)];
    auto& starting = result.sets_[static_cast<exclusion_set::id>(e_id * 3 + 1)];
    auto& ending = result.sets_[static_cast<exclusion_set::id>(e_id * 3 + 2)];

    closed = make_exclusion_set(ir_ids);
    starting = closed;
    ending = closed;

    auto const e = infra->graph_.elements_[e_id];
    if (!interlocking_route::valid_end(e->type())) {
      continue;
    }

    closed.clear();

    auto const n_id = e->as<track_element>().get_node()->id_;
    starting -= make_exclusion_set(ending_at[n_id]);
    ending -= make_exclusion_set(starting_at[n_id]);
  }

  // compact the layout by removing empty sets
  exclusion_set::id current_id = 0;
  for (auto i = 0U; i < element_count; ++i) {

    if (auto& closed = result.sets_[i * 3]; !closed.empty()) {  // NOLINT
      result.element_to_exclusion_set_[i].closed_ =
          exclusion_set::optional_id{current_id};
      closed.id_ = current_id;
      ++current_id;
    }

    if (auto& start = result.sets_[i * 3 + 1]; !start.empty()) {  // NOLINT
      result.element_to_exclusion_set_[i].starting_ =
          exclusion_set::optional_id{current_id};
      start.id_ = current_id;
      ++current_id;
    }

    if (auto& ending = result.sets_[i * 3 + 2]; !ending.empty()) {  // NOLINT
      result.element_to_exclusion_set_[i].ending_ =
          exclusion_set::optional_id{current_id};
      ending.id_ = current_id;
      ++current_id;
    }
  }

  utl::erase_if(result.sets_,
                [&](auto&& es) { return es.id_ == exclusion_set::INVALID_ID; });

  uLOG(utl::info) << "created " << result.sets_.size()
                  << " initial element exclusion sets.";

  utls::ensures([&]() {
    for (auto const& es_ids : result.element_to_exclusion_set_) {
      utls::ensure(!es_ids.closed_.has_value() ||
                   *es_ids.closed_ < result.sets_.size());
      utls::ensure(!es_ids.starting_.has_value() ||
                   *es_ids.starting_ < result.sets_.size());
      utls::ensure(!es_ids.ending_.has_value() ||
                   *es_ids.ending_ < result.sets_.size());
    }
  });
  utls::ensure(
      result.element_to_exclusion_set_.size() == infra->graph_.elements_.size(),
      "valid element to optional exclusion set mapping");

  return result;
}

working_set get_working_set(
    element_id const starting_element,
    interlocking_route::ids const& starting_at,
    exclusion_sets const& exclusion_sets,
    soro::vector<element::ids> const& open_exclusion_elements,
    infrastructure const& infra) {

  working_set ws;

  for (auto const ir_id : starting_at) {
    for (auto const& ee_id : open_exclusion_elements[ir_id]) {
      auto const& ee = infra->graph_.elements_[ee_id];

      auto const& es_ids = exclusion_sets.element_to_exclusion_set_[ee_id];

      if (interlocking_route::valid_end(ee->type())) {
        utls::sassert(es_ids.starting_.has_value());
        utls::sassert(es_ids.ending_.has_value());
        ws.exclusion_sets_.emplace_back(es_ids.starting_.value());
        ws.exclusion_sets_.emplace_back(es_ids.ending_.value());
      } else {
        utls::sassert(es_ids.closed_.has_value());
        ws.exclusion_sets_.emplace_back(es_ids.closed_.value());
      }
    }

    auto const& ir = infra->interlocking_.routes_[ir_id];

    auto const first = ir.first_node(infra)->element_;
    auto const first_es_ids =
        exclusion_sets.element_to_exclusion_set_[first->id()];

    utls::sassert(first_es_ids.starting_.has_value());
    ws.exclusion_sets_.emplace_back(first_es_ids.starting_.value());

    auto const last = ir.last_node(infra)->element_->id();
    auto const last_es_ids = exclusion_sets.element_to_exclusion_set_[last];

    utls::sassert(last_es_ids.ending_.has_value());
    ws.exclusion_sets_.emplace_back(last_es_ids.ending_.value());
  }

  utl::erase_duplicates(ws.exclusion_sets_);
  ws.stems_from_ = starting_element;

  utls::ensures([&]() {
    for (auto const es_id : ws.exclusion_sets_) {
      utls::ensure(es_id < exclusion_sets.sets_.size(),
                   "invalid exclusion set id");
    }
  });

  return ws;
}

soro::vector<working_set> get_working_sets(
    exclusion_sets const& exclusion_sets,
    soro::vector<element::ids> const& open_exclusion_elements,
    infrastructure const& infra) {
  utl::scoped_timer const timer("generating working sets");

  soro::vector<working_set> result;

  for (auto const [n_id, starting_at] :
       utl::enumerate(infra->interlocking_.starting_at_)) {
    if (starting_at.empty()) {
      continue;
    }

    auto const starting_element =
        infra->graph_.nodes_[static_cast<node::id>(n_id)]->element_->id();

    result.emplace_back(get_working_set(starting_element, starting_at,
                                        exclusion_sets, open_exclusion_elements,
                                        infra));
  }

  return result;
}

exclusion_set::ids get_used_exclusion_sets(
    soro::vector<working_set> const& working_sets) {
  using namespace ranges;

  return working_sets |
         views::transform([](auto&& ws) { return ws.exclusion_sets_; }) |
         views::join | to<std::vector> | actions::sort | actions::unique;
}

soro::vector<exclusion_set> normalize_exclusion_sets(
    soro::vector<working_set> const& working_sets,
    soro::vector<exclusion_set> const& exclusion_sets) {
  utl::scoped_timer const timer("normalizing exclusion sets");
  using namespace ranges;

  auto const used_exclusion_set_ids = get_used_exclusion_sets(working_sets);

  auto used_exclusion_sets =
      used_exclusion_set_ids |
      views::transform([&](auto&& es_id) { return exclusion_sets[es_id]; }) |
      to<soro::vector<exclusion_set>>();

  for (auto [es_id, es] : utl::enumerate(used_exclusion_sets)) {
    es.id_ = static_cast<exclusion_set::id>(es_id);
  }

  return used_exclusion_sets;
}

void clean_up_working_sets(soro::vector<exclusion_set> const& exclusion_sets,
                           soro::vector<working_set>& working_sets) {
  utl::scoped_timer const timer("cleaning up working sets");

  soro::vector<exclusion_set::id> replacement_set(exclusion_sets.size());
  std::iota(std::begin(replacement_set), std::end(replacement_set), 0);

  for (auto const& working_set : working_sets) {
    if (working_set.empty()) {
      continue;
    }

    for (auto i = 0U; i < working_set.size(); ++i) {
      for (auto j = i + 1; j < working_set.size(); ++j) {
        auto const s1_id = working_set.exclusion_sets_[i];
        auto const s2_id = working_set.exclusion_sets_[j];

        auto const s1_replacement = replacement_set[s1_id];
        auto const s2_replacement = replacement_set[s2_id];

        auto const& s1 = exclusion_sets[s1_replacement];
        auto const& s2 = exclusion_sets[s2_replacement];

        if (s1.empty() || s2.empty() || i == j ||
            s1_replacement == s2_replacement) {
          continue;
        }

        auto const comparison = s1.compare(s2);

        if (comparison == std::partial_ordering::less) {
          replacement_set[s1_id] = s2_replacement;
          replacement_set[s1_replacement] = s2_replacement;
          continue;
        }

        if (comparison == std::partial_ordering::greater) {
          replacement_set[s2_id] = s1_replacement;
          replacement_set[s2_replacement] = s1_replacement;
          continue;
        }

        if (comparison == std::partial_ordering::equivalent) {
          auto const chosen_one = std::min(s1_replacement, s2_replacement);
          replacement_set[s1_id] = chosen_one;
          replacement_set[s2_id] = chosen_one;
          replacement_set[s1_replacement] = chosen_one;
          replacement_set[s2_replacement] = chosen_one;
          continue;
        }
      }
    }
  }

  timer.print("replacing and removing duplicates/empties");

  for (auto& working_set : working_sets) {
    for (auto& i : working_set.exclusion_sets_) {
      i = replacement_set[i];
    }

    utl::erase_duplicates(working_set.exclusion_sets_);
    utl::erase_if(working_set.exclusion_sets_,
                  [&](auto&& es_id) { return exclusion_sets[es_id].empty(); });
  }
}

void expand_working_set(
    working_set& working_set, exclusion_sets const& exclusion_sets,
    soro::vector<element::ids> const& open_exclusion_elements) {

  // or-combine all exclusion sets in the working set
  // this yields an exclusion set that contains all used IRs in this working set
  auto const or_combine = [&](auto&& es, auto&& es_id) {
    es |= exclusion_sets.sets_[es_id];
    return std::forward<decltype(es)>(es);
  };

  auto const all_used_irs = utls::accumulate(
      working_set.exclusion_sets_, make_exclusion_set({}), or_combine);

  utls::sasserts([&]() {
    for (auto const& es_id : working_set.exclusion_sets_) {
      utls::sassert(all_used_irs.contains(exclusion_sets.sets_[es_id]));
    }
  });

  // remove the IRs that start at the element that induces the working set
  auto const es_ids =
      exclusion_sets.element_to_exclusion_set_[working_set.stems_from_];
  utls::sassert(es_ids.starting_.has_value(),
                "no exclusion set for working set stem");
  auto const& starting_exclusion_set = exclusion_sets.sets_[*es_ids.starting_];

  auto const additional_irs = all_used_irs - starting_exclusion_set;

  // actually expand the working set with exclusion sets from IRs
  // that touch IRs currently in the working set
  for (auto const ir_id : additional_irs) {
    for (auto const& ee : open_exclusion_elements[ir_id]) {
      if (exclusion_sets.element_to_exclusion_set_[ee].closed_.has_value()) {
        working_set.exclusion_sets_.emplace_back(
            exclusion_sets.element_to_exclusion_set_[ee].closed_.value());
      }
    }
  }

  utl::erase_duplicates(working_set.exclusion_sets_);
}

void expand_working_sets(
    soro::vector<working_set>& working_sets, exclusion_sets const& sets,
    soro::vector<element::ids> const& open_exclusion_elements) {
  utl::scoped_timer const timer("expanding working sets");

  utl::parallel_for_run(working_sets.size(), [&](auto&& job_id) {
    expand_working_set(working_sets[job_id], sets, open_exclusion_elements);
  });
}

void work_on_working_set(working_set const& working_set,
                         soro::vector<exclusion_set> const& ro_exclusion_sets,
                         soro::vector<exclusion_set>& exclusion_sets,
                         std::vector<std::mutex>& exclusion_set_mutexes) {
  if (working_set.exclusion_sets_.size() <= 2) {
    return;
  }

  exclusion_set all;
  for (auto const es_id : working_set.exclusion_sets_) {
    all |= ro_exclusion_sets[es_id];
  }

  std::unordered_map<interlocking_route::id, exclusion_set> ir_all_sets;

  for (auto const ir_id : all) {
    exclusion_set ir_all_set;

    for (auto const& es_id : working_set.exclusion_sets_) {
      if (ro_exclusion_sets[es_id][ir_id]) {
        ir_all_set |= ro_exclusion_sets[es_id];
      }
    }

    ir_all_sets.emplace(ir_id, std::move(ir_all_set));
  }

  auto const can_be_added_to =
      [](interlocking_route::id const ir_id, exclusion_set::id const& dest,
         struct working_set const& ws,
         soro::vector<exclusion_set> const& sets) -> bool {
    auto need_to_check = sets[dest];

    if (dest == 46) {
      std::ignore = need_to_check;
    }

    if (ir_id == 37 && dest == 46) {
      std::ignore = need_to_check;
    }

    for (auto const es_id : ws.exclusion_sets_) {
      auto const& es = sets[es_id];

      if (!es[ir_id]) {
        continue;
      }

      need_to_check -= es;
    }

    if (dest == 46 && need_to_check.empty()) {
      std::ignore = need_to_check;
    }

    return need_to_check.empty();
  };

  for (auto const es_id : working_set.exclusion_sets_) {
    auto const todo = all - ro_exclusion_sets[es_id];

    for (auto const ir_id : todo) {
      if (can_be_added_to(ir_id, es_id, working_set, ro_exclusion_sets)) {
        std::lock_guard<std::mutex> const lock(exclusion_set_mutexes[es_id]);
        exclusion_sets[es_id].set(ir_id);
      }
    }
  }
}

void work_on_working_sets(soro::vector<working_set> const& working_sets,
                          soro::vector<exclusion_set>& exclusion_sets) {
  utl::scoped_timer const timer("working on working sets");

  // const read only copy
  auto const ro_sets = exclusion_sets;
  std::vector<std::mutex> exclusion_set_mutexes(exclusion_sets.size());

  utl::parallel_for_run(working_sets.size(), [&](auto&& ws_id) {
    work_on_working_set(working_sets[ws_id], ro_sets, exclusion_sets,
                        exclusion_set_mutexes);
  });
}

soro::vector<exclusion_set> get_exclusion_sets(
    infrastructure const& infra,
    soro::vector<interlocking_route::ids> const& closed_element_used_by,
    soro::vector<element::ids> const& open_exclusion_elements) {

  auto exclusion_sets = get_initial_exclusion_sets(
      closed_element_used_by, infra->interlocking_.starting_at_,
      infra->interlocking_.ending_at_, infra);

  auto working_sets =
      get_working_sets(exclusion_sets, open_exclusion_elements, infra);
  clean_up_working_sets(exclusion_sets.sets_, working_sets);

  expand_working_sets(working_sets, exclusion_sets, open_exclusion_elements);
  clean_up_working_sets(exclusion_sets.sets_, working_sets);

  work_on_working_sets(working_sets, exclusion_sets.sets_);
  clean_up_working_sets(exclusion_sets.sets_, working_sets);

  uLOG(utl::info) << "cleaning up all exclusion sets";
  working_set all_used;
  all_used.exclusion_sets_ = get_used_exclusion_sets(working_sets);
  soro::vector<working_set> all_v = {all_used};
  clean_up_working_sets(exclusion_sets.sets_, all_v);

  return normalize_exclusion_sets(all_v, exclusion_sets.sets_);
}

}  // namespace soro::infra
