#include "rapid/network.h"

#include <optional>

#include "fmt/color.h"

#include "cista/containers/hash_set.h"

#include "utl/verify.h"

namespace cr = cista::raw;

namespace rapid {

std::ostream& operator<<(std::ostream& out, pixel_pos const& p) {
  return out << p.x_ << "|" << p.y_;
}

void network::print(std::vector<edge*> const& highlight_edges) const {
  cr::hash_set<pixel_pos> highlight;
  auto const add_highlight = [&](auto const& el) {
    if (el != nullptr) {
      for (auto const& r : el->draw_representation_) {
        highlight.emplace(r.pos_);
      }
    }
  };
  for (auto const& e : highlight_edges) {
    add_highlight(e);
    add_highlight(e->from_);
    add_highlight(e->to_);
  }

  struct draw {
    char c_{' '};
    std::optional<fmt::color> color_;
  };

  std::vector<std::vector<draw>> lines;
  auto const add = [&](pixel const pix) {
    assert(pix.pos_.valid());
    auto const [p, c] = pix;
    if (lines.size() <= static_cast<size_t>(p.y_)) {
      lines.resize(p.y_ + 1U);
    }
    if (lines[p.y_].size() <= static_cast<size_t>(p.x_)) {
      lines[p.y_].resize(p.x_ + 1U);
    }
    lines[p.y_][p.x_] = {c, highlight.find(pix.pos_) != end(highlight)
                                ? std::make_optional(fmt::color::red)
                                : std::nullopt};
  };
  for (auto const& n : nodes_) {
    for (auto const& d : n->draw_representation_) {
      add(d);
    }
  }
  for (auto const& e : edges_) {
    for (auto const& d : e->draw_representation_) {
      add(d);
    }
  }
  for (auto const& l : lines) {
    for (auto const& c : l) {
      if (c.color_.has_value()) {
        fmt::print(fmt::fg(*c.color_) | fmt::emphasis::bold, "{}", c.c_);
      } else {
        fmt::print("{}", c.c_);
      }
    }
    fmt::print("\n");
  }
}

std::ostream& operator<<(std::ostream& out, network const& n) {
  n.print();
  return out;
}

std::string_view type_str(node::type const t) {
  static char const* types[] = {"END_NODE",      "APPROACH_SIGNAL",
                                "MAIN_SIGNAL",   "END_OF_TRAIN_DETECTOR",
                                "SWITCH",        "SINGLE_SLIP_SWITCH",
                                "LEVEL_JUNCTION"};
  return types[static_cast<std::underlying_type_t<node::type>>(t)];
}

std::ostream& operator<<(std::ostream& out, node::type const t) {
  return out << type_str(t);
}

void edge::add_node(node* n) {
  if (from_ == n || to_ == n) {
    return;
  } else if (from_ == nullptr) {
    from_ = n;
  } else if (to_ == nullptr) {
    to_ = n;
  } else {
    utl::verify(false, "edge with >2 nodes");
  }
}

node* edge::opposite(node* n) const { return n == from_ ? to_ : from_; }

}  // namespace rapid