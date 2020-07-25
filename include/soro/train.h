#pragma once

#include <ctime>
#include <iosfwd>
#include <vector>

#include "cista/containers/hash_map.h"
#include "cista/containers/hash_set.h"
#include "cista/containers/unique_ptr.h"
#include "cista/reflection/comparable.h"

#include "soro/dpd.h"
#include "soro/speed_t.h"
#include "soro/unixtime.h"

namespace soro {

struct node;
struct edge;
struct network;
struct train;

struct Granularity {
  template <size_t Index>
  constexpr size_t get();

  template <>
  constexpr size_t get<0>() {
    return 10;
  }

  template <>
  constexpr size_t get<1>() {
    return 20;
  }
};

struct route {
  struct id_t {
    CISTA_COMPARABLE()
    node *from_, *to_;
  };

  std::vector<std::string> warnings() const;
  std::string tag() const;
  bool finished() const;
  void compute_dists();
  void compute_sched_times();
  bool ready() const;
  id_t id() const;

  friend std::ostream& operator<<(std::ostream&, route const&);

  node* approach_signal_{nullptr};  // contained in previous route
  node* end_of_train_detector_{nullptr};  // contained in next route
  node *from_{nullptr}, *to_{nullptr};
  route *pred_{nullptr}, *succ_{nullptr};
  unixtime from_time_{0}, to_time_{0};
  std::vector<edge*> path_;
  train* train_{nullptr};
  cista::raw::hash_set<route*> in_, out_;
  unsigned dist_to_eotd_{0U}, total_dist_{0U};
  dpd<Granularity, unixtime, speed_t> entry_dpd_;
  dpd<Granularity, unixtime, speed_t> exit_dpd_;
  dpd<Granularity, unixtime> eotd_dpd_;
};

struct train {
  struct timetable_entry {
    unixtime time_;
    node* node_;
  };

  void build_routes(network const&);
  friend std::ostream& operator<<(std::ostream&, train const&);

  unixtime arrival_time(unixtime const start, unsigned total_dist) {
    return start +
           unixtime{static_cast<time_t>((static_cast<float>(total_dist) /
                                         (static_cast<float>(speed_) / 100.0)) *
                                        60)};
  }

  std::string name_;
  speed_t speed_;
  std::vector<timetable_entry> timetable_;
  std::vector<std::unique_ptr<route>> routes_;
};

using timetable =
    cista::raw::hash_map<std::string, cista::raw::unique_ptr<train>>;

}  // namespace soro
