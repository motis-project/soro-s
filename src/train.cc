#include "soro/train.h"

#include <numeric>
#include <ostream>

#include "utl/enumerate.h"
#include "utl/pairwise.h"
#include "utl/verify.h"

#include "date/date.h"

#include "soro/dijkstra.h"
#include "soro/network.h"
#include "soro/time_util.h"

namespace soro {

std::ostream& operator<<(std::ostream& out, route const& r) {
  return out << "{ train=" << r.train_->name_ << ", "
             << (r.from_ == nullptr ? "" : r.from_->name_) << "@"
             << r.from_time_ << " -> " << r.to_->name_ << "@" << r.to_time_
             << " }";
}

std::string route::tag() const {
  return from_ == nullptr
             ? train_->name_ + "_START"
             : train_->name_ + "_" + from_->name_ + "_" + to_->name_;
}

bool route::finished() const { return !entry_dpd_.empty(); }

bool route::ready() const {
  return std::all_of(begin(in_), end(in_),
                     [](route* in) { return in->finished(); });
}

std::vector<std::string> route::warnings() const {
  std::vector<std::string> warnings;
  if (approach_signal_ == nullptr) {
    warnings.emplace_back("route without approach signal");
  }
  if (end_of_train_detector_ == nullptr) {
    warnings.emplace_back("route without end of train detector");
  }
  if (from_ == nullptr) {
    warnings.emplace_back("route without from");
  }
  if (to_ == nullptr) {
    warnings.emplace_back("route without to");
  }
  return warnings;
}

void train::build_routes(network const& net) {
  route start;
  start.train_ = this;
  start.entry_dpd_ = start.exit_dpd_ =
      decltype(start.entry_dpd_){timetable_.front().time_, speed_t{speed_}};
  start.eotd_dpd_ = decltype(start.eotd_dpd_){timetable_.front().time_};
  start.to_ = timetable_.front().node_;
  auto pred = routes_.emplace_back(std::make_unique<route>(start)).get();

  for (auto const [source, dest] : utl::pairwise(timetable_)) {
    auto const edges = dijkstra(net, source.node_, dest.node_);
    utl::verify(!edges.empty(), "path for {} from {} to {} not found", name_,
                source.node_->name_, dest.node_->name_);
    node* curr_node{source.node_};
    route curr_route, next_route;
    curr_route.from_ = source.node_;
    for (auto const& e : edges) {
      curr_route.path_.emplace_back(e);
      auto const edge_to = e->opposite(curr_node);
      switch (edge_to->type_) {
        case node::type::APPROACH_SIGNAL:
          next_route.approach_signal_ = e->to_;
          break;

        case node::type::MAIN_SIGNAL:
          curr_route.to_ = edge_to;
          curr_route.train_ = this;
          pred->succ_ =
              routes_.emplace_back(std::make_unique<route>(curr_route)).get();
          pred->succ_->pred_ = pred;
          pred = pred->succ_;
          curr_route = next_route;
          curr_route.from_ = edge_to;
          break;

        case node::type::END_OF_TRAIN_DETECTOR:
          if (!routes_.empty()) {
            curr_route.dist_to_eotd_ = std::accumulate(
                cbegin(curr_route.path_), cend(curr_route.path_), 0U,
                [](unsigned dist, edge const* a) { return dist + a->dist_; });
            routes_.back()->end_of_train_detector_ = e->to_;
          }
          break;

        default:;
      }
      if (edge_to == dest.node_) {
        curr_route.to_ = dest.node_;
        curr_route.train_ = this;
        pred->succ_ =
            routes_.emplace_back(std::make_unique<route>(curr_route)).get();
        pred->succ_->pred_ = pred;
      }
      curr_node = edge_to;
    }

    auto time = source.time_;
    for (auto& r : routes_) {
      r->from_time_ = time;
      auto const total_dist = std::accumulate(
          cbegin(r->path_), cend(r->path_), 0U,
          [](unsigned dist, edge const* e) { return dist + e->dist_; });
      time = r->train_->arrival_time(time, total_dist);
      r->total_dist_ = total_dist;
      r->to_time_ = time;
    }
  }
}

void route::compute_dists() {
  unixtime start{std::numeric_limits<unixtime>::min()};
  for (auto const& in : in_) {
    if (in->train_ == train_) {
      start = std::max(start, in->exit_dpd_.first_);
    } else {
      start = std::max(start, in->eotd_dpd_.first_);
    }
  }
  entry_dpd_ = decltype(entry_dpd_){start, train_->speed_};
  eotd_dpd_ = decltype(eotd_dpd_){train_->arrival_time(start, dist_to_eotd_)};
  exit_dpd_ = decltype(exit_dpd_){train_->arrival_time(start, total_dist_),
                                  train_->speed_};
}

std::ostream& operator<<(std::ostream& out, train const& t) {
  out << "{\n  name = " << t.name_ << ",\n  speed = " << t.speed_
      << ",\n  timetable = [\n";
  for (auto const [i, entry] : utl::enumerate(t.timetable_)) {
    out << "    " << entry.node_->name_ << " "
        << date::format("%F %T",
                        date::sys_seconds{std::chrono::seconds{entry.time_}});
    if (i != t.timetable_.size() - 1) {
      std::cout << ",";
    }
    std::cout << "\n";
  }
  std::cout << "  ],\n  routes = [\n";
  for (auto const& r : t.routes_) {
    std::cout << "    " << (r->from_ == nullptr ? "" : r->from_->name_) << "@"
              << r->from_time_ << " -> " << r->to_->name_ << "@" << r->to_time_
              << ",\n";
  }
  return out << "  ]\n}";
}

}  // namespace soro