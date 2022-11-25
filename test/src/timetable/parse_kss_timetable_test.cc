#include "doctest/doctest.h"

#include "soro/utls/algo/overlap.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/interlocking/exclusion.h"
#include "soro/timetable/timetable.h"

#include "test/file_paths.h"

using namespace soro;
using namespace soro::tt;
using namespace soro::infra;

TEST_SUITE("KSS") {
  TEST_CASE("parse simple kss") {}
}

struct sub_station_route {
  using sub_id = uint8_t;
  using id = uint32_t;

  id id_;
  station_route::id sr_id_;
  station::id station_id_;
  sub_id sub_id_;

  node::idx from_;
  node::idx to_;
  std::vector<node::ptr> sorted_nodes_;
};

bool excludes(sub_station_route const& sr1, sub_station_route const& sr2) {
  // TODO(julian) this could be improved by only checking for overlapping
  // sections, instead of overlapping nodes
  return utls::overlap(sr1.sorted_nodes_, sr2.sorted_nodes_);
}

std::vector<sub_station_route> get_sub_station_routes(
    soro::vector<station_route::ptr> const& station_routes) {
  std::vector<sub_station_route> result;
  result.reserve(station_routes.size());

  sub_station_route::id current_id = 0;

  for (auto const& sr : station_routes) {

    if (sr->path_->main_signals_.empty()) {
      sub_station_route ssr;
      ssr.id_ = current_id++;
      ssr.sr_id_ = sr->id_;
      ssr.station_id_ = sr->station_->id_;
      ssr.from_ = 0;
      ssr.to_ = sr->size();
      ssr.sub_id_ = 0;
      ssr.sorted_nodes_ = sr->nodes();
      utls::sort(ssr.sorted_nodes_);

      result.emplace_back(ssr);

      continue;
    }

    if (sr->path_->main_signals_.size() == 1) {
      {
        sub_station_route ssr1;
        ssr1.id_ = current_id++;
        ssr1.sr_id_ = sr->id_;
        ssr1.station_id_ = sr->station_->id_;
        ssr1.from_ = 0;
        ssr1.to_ = sr->path_->main_signals_.front() + 1;
        ssr1.sub_id_ = 0;
        ssr1.sorted_nodes_.insert(
            end(ssr1.sorted_nodes_), begin(sr->nodes()),
            begin(sr->nodes()) + sr->path_->main_signals_.front());
        utls::sort(ssr1.sorted_nodes_);
        result.emplace_back(ssr1);
      }

      {
        sub_station_route ssr2;
        ssr2.id_ = current_id++;
        ssr2.sr_id_ = sr->id_;
        ssr2.station_id_ = sr->station_->id_;
        ssr2.from_ = sr->path_->main_signals_.front();
        ssr2.to_ = sr->nodes().size();
        ssr2.sub_id_ = 1;
        ssr2.sorted_nodes_.insert(
            end(ssr2.sorted_nodes_),
            begin(sr->nodes()) + sr->path_->main_signals_.front(),
            end(sr->nodes()));
        utls::sort(ssr2.sorted_nodes_);
        result.emplace_back(ssr2);
      }

      continue;
    }

    if (sr->path_->main_signals_.size() > 1) {
      sub_station_route::sub_id sub_id = 0;

      {
        sub_station_route ssr1;
        ssr1.id_ = current_id++;
        ssr1.sr_id_ = sr->id_;
        ssr1.station_id_ = sr->station_->id_;
        ssr1.from_ = 0;
        ssr1.to_ = sr->path_->main_signals_.front() + 1;
        ssr1.sub_id_ = sub_id++;
        ssr1.sorted_nodes_.insert(
            end(ssr1.sorted_nodes_), begin(sr->nodes()),
            begin(sr->nodes()) + sr->path_->main_signals_.front());
        utls::sort(ssr1.sorted_nodes_);
        result.emplace_back(ssr1);
      }

      for (auto const [from, to] : utl::pairwise(sr->path_->main_signals_)) {
        sub_station_route ssr;
        ssr.id_ = current_id++;
        ssr.sr_id_ = sr->id_;
        ssr.station_id_ = sr->station_->id_;
        ssr.from_ = from;
        ssr.to_ = to;
        ssr.sub_id_ = sub_id++;
        ssr.sorted_nodes_.insert(end(ssr.sorted_nodes_),
                                 begin(sr->nodes()) + from,
                                 begin(sr->nodes()) + to);
        utls::sort(ssr.sorted_nodes_);

        result.emplace_back(ssr);
      }

      {
        sub_station_route ssr2;
        ssr2.id_ = current_id++;
        ssr2.sr_id_ = sr->id_;
        ssr2.station_id_ = sr->station_->id_;
        ssr2.from_ = sr->path_->main_signals_.front();
        ssr2.to_ = sr->nodes().size();
        ssr2.sub_id_ = sub_id++;
        ssr2.sorted_nodes_.insert(
            end(ssr2.sorted_nodes_),
            begin(sr->nodes()) + sr->path_->main_signals_.back(),
            end(sr->nodes()));
        utls::sort(ssr2.sorted_nodes_);
        result.emplace_back(ssr2);
      }

      continue;
    }
  }

  return result;
}

auto get_sub_route_exclusion(sub_station_route const& ssr,
                             std::vector<sub_station_route> const& candidates,
                             infrastructure const&) {
  std::vector<sub_station_route::id> exclusions;

  for (auto const& candidate : candidates) {
    if (candidate.id_ <= ssr.id_) {
      continue;
    }

    if (excludes(candidate, ssr)) {
      exclusions.emplace_back(candidate.id_);
    }
  }

  exclusions.emplace_back(ssr.id_);

  return exclusions;
}

auto get_station_to_sub_routes(std::vector<sub_station_route> const& ssrs,
                               infrastructure const& infra) {
  std::vector<std::vector<sub_station_route>> result(infra->stations_.size());

  for (auto const& ssr : ssrs) {
    result[infra->station_routes_[ssr.sr_id_]->station_->id_].emplace_back(ssr);
  }
  return result;
}

auto get_sub_route_exclusions(infrastructure const& infra) {

  auto const ssrs = get_sub_station_routes(infra->station_routes_);
  auto const station_to_sub_routes = get_station_to_sub_routes(ssrs, infra);

  std::vector<std::vector<sub_station_route::id>> exclusions(ssrs.size());

  for (auto const& ssr : ssrs) {
    exclusions[ssr.id_] = get_sub_route_exclusion(
        ssr, station_to_sub_routes[ssr.station_id_], infra);
  }

  return exclusions;
}

template <typename VecVec>
void print_vecvec_stats(VecVec const& vecvec) {
  std::size_t avg = 0;
  std::size_t max = 0;
  std::size_t min = 9999;

  std::vector<std::size_t> sizes;

  for (auto const& vec : vecvec) {
    avg += vec.size();
    min = std::min(min, vec.size());
    max = std::max(max, vec.size());
    sizes.emplace_back(vec.size());
  }

  utls::sort(sizes);

  std::cout << "Min: " << min << " Max: " << max
            << " Avg: " << avg / vecvec.size() << std::endl;

  auto const get_quantile = [](double const q,
                               std::vector<std::size_t> const& v) {
    return v[static_cast<std::size_t>((static_cast<double>(v.size()) * q))];
  };

  std::cout << "99%: " << get_quantile(0.99, sizes) << '\n';
  std::cout << "95%: " << get_quantile(0.95, sizes) << '\n';
  std::cout << "90%: " << get_quantile(0.90, sizes) << '\n';
  std::cout << "50%: " << get_quantile(0.50, sizes) << '\n';
}

bool same_path(station_route::ptr const sr1, station_route::ptr const sr2) {
  if (sr1->path_->start_ != sr2->path_->start_ ||
      sr1->path_->end_ != sr2->path_->end_) {
    return false;
  }

  if (sr1->path_->course_ != sr2->path_->course_) {
    return false;
  }

  if (sr1->path_->main_signals_ != sr2->path_->main_signals_) {
    return false;
  }

  return true;
}

void same_path_station_routes(infrastructure const& infra) {
  std::size_t same_path_count = 0;

  for (auto const station : infra->stations_) {
    for (auto const& [_1, sr1] : station->station_routes_) {
      for (auto const& [_2, sr2] : station->station_routes_) {

        if (sr1 == sr2) {
          continue;
        }

        if (same_path(sr1, sr2)) {
          ++same_path_count;
        }
      }
    }
  }

  std::cout << "same paths: " << (same_path_count / 2) << std::endl;
}

TEST_SUITE("parse de_kss" *
           doctest::skip(!std::filesystem::exists(DE_KSS_FOLDER))) {

  TEST_CASE("parse de_kss") {
    auto opts = DE_ISS_OPTS;
    opts.determine_interlocking_ = false;
    opts.determine_conflicts_ = false;
    infrastructure const infra(opts);

    auto const exclusion_path_exclusions = get_exclusion_path_exclusions(infra);
    print_vecvec_stats(exclusion_path_exclusions);

    //    same_path_station_routes(infra);
    //
    //    auto const ssrs_exclusion = get_sub_route_exclusions(infra);
    //
    //    std::size_t id = 0;
    //    for (auto const& ssr_exclusions : ssrs_exclusion) {
    //
    //      if (ssr_exclusions.size() == 1850) {
    //        std::set<sub_station_route::id> s;
    //        std::set<std::pair<sub_station_route::id, sub_station_route::id>>
    //        e;
    //        //
    //        for (auto const& other_ss : ssr_exclusions) {
    //          e.insert({id, other_ss});
    //
    //          for (auto const& other_ex : ssrs_exclusion[other_ss]) {
    //            e.insert({other_ss, other_ex});
    //            s.insert(other_ex);
    //          }
    //        }
    //        //
    //        std::cout << "set size: " << s.size() << '\n';
    //        std::cout << "edges: " << e.size() << '\n';
    //        std::cout << "ID: " << id << '\n';
    //      }
    //
    //      ++id;
    //    }
    //
    //    print_vecvec_stats(ssrs_exclusion);

    //    infra.save("infra.raw");
    //    timetable const tt(DE_KSS_OPTS, infra);

    //    std::cout << "Got " << tt->train_store_.size() << " trains" <<
    //    std::endl; tt.save("tt.raw");
  }
}
