#pragma once

#include <cinttypes>
#include <limits>
#include <ostream>
#include <set>
#include <string>
#include <vector>

#include "cista/containers/hash_map.h"
#include "cista/containers/hash_set.h"
#include "cista/containers/vector.h"

namespace rapid {

using pixel_coord_t = uint32_t;
using distance_in_m = uint32_t;

struct pixel_pos {
  static constexpr auto const INVALID =
      std::numeric_limits<pixel_coord_t>::max();
  bool valid() const { return x_ != INVALID && y_ != INVALID; }
  friend std::ostream& operator<<(std::ostream&, pixel_pos const&);
  pixel_coord_t x_{INVALID}, y_{INVALID};
};

struct pixel {
  pixel_pos pos_;
  char content_;
};

struct node;

struct station {
  unsigned id_;
  std::string name_;
};

struct edge {
  void add_node(node*);

  node* opposite(node*) const;
  unsigned id_{0U};
  node *from_{nullptr}, *to_{nullptr};
  distance_in_m dist_{0U};
  cista::raw::vector<pixel> draw_representation_;
};

struct node {
  enum class type : uint8_t {
    END_NODE,
    APPROACH_SIGNAL,
    MAIN_SIGNAL,
    END_OF_TRAIN_DETECTOR,
    SWITCH,
    SINGLE_SLIP_SWITCH,
    LEVEL_JUNCTION
  } type_;

  station* station_;
  std::string name_;
  cista::raw::hash_map<edge*, cista::raw::hash_set<edge*>> traversals_;
  cista::raw::hash_map<edge*, edge*> action_traversals_;
  edge* end_node_edge_;
  cista::raw::vector<pixel> draw_representation_;
};

std::string_view type_str(node::type const t);

std::ostream& operator<<(std::ostream&, node::type const);

struct network {
  void print(std::vector<edge*> const& highlight_edges = {}) const;
  friend std::ostream& operator<<(std::ostream&, network const&);
  std::vector<std::unique_ptr<node>> nodes_;
  std::vector<std::unique_ptr<edge>> edges_;
  std::vector<std::unique_ptr<station>> stations_;
};

}  // namespace rapid