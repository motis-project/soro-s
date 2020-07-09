#include "rapid/ascii_network_parser.h"

#include <iostream>
#include <optional>
#include <sstream>

#include "cista/containers/hash_map.h"
#include "cista/containers/hash_set.h"
#include "cista/containers/pair.h"
#include "cista/containers/variant.h"

#include "utl/enumerate.h"
#include "utl/erase_if.h"
#include "utl/overloaded.h"
#include "utl/parser/arg_parser.h"
#include "utl/parser/cstr.h"
#include "utl/pipes.h"
#include "utl/verify.h"

namespace rapid {

namespace cr = cista::raw;

struct ascii_network_parser {
  using map_el_t = cr::variant<node*, edge*>;

  enum type : char {
    EMPTY = ' ',

    // Tracks.
    HORIZONTAL = '=',
    VERTICAL = '|',
    DIAGONAL_LR = '\\',
    DIAGONAL_RL = '/',

    // Signal direction indicators.
    DIRECTION_RIGHT = '>',
    DIRECTION_LEFT = '<',
    DIRECTION_TOP = ';',
    DIRECTION_BOTTOM = '!',

    // End of train detectors.
    END_OF_TRAIN_DETECTOR_L = '[',
    END_OF_TRAIN_DETECTOR_R = ']',

    // Approach signals.
    APPROACH_SIGNAL_L = '(',
    APPROACH_SIGNAL_R = ')',

    // Direction change / switch
    KNOT = '*',

    // Level junction (no difference)
    LEVEL_JUNCTION = '#',
    LEVEL_JUNCTION_ALT = 'X',

    // Single slip switch.
    SINGLE_SLIP = '^',
    SINGLE_SLIP_INVERTED = 'v'
  };

  enum class dir {
    TOP_LEFT,
    TOP,
    TOP_RIGHT,
    LEFT,
    RIGHT,
    BOTTOM_LEFT,
    BOTTOM,
    BOTTOM_RIGHT
  };

  struct signal_info {
    type direction_;
    std::vector<pixel_pos> positions_;
  };

  static constexpr pixel_pos next(pixel_pos const p, dir const d) {
    switch (d) {
      case dir::TOP_LEFT: return {p.x_ - 1, p.y_ - 1};
      case dir::TOP: return {p.x_, p.y_ - 1};
      case dir::TOP_RIGHT: return {p.x_ + 1, p.y_ - 1};
      case dir::LEFT: return {p.x_ - 1, p.y_};
      case dir::RIGHT: return {p.x_ + 1, p.y_};
      case dir::BOTTOM_LEFT: return {p.x_ - 1, p.y_ + 1};
      case dir::BOTTOM: return {p.x_, p.y_ + 1};
      case dir::BOTTOM_RIGHT: return {p.x_ + 1, p.y_ + 1};
    }
    assert(false);
    return p;
  }

  static constexpr type get_orientation(dir const d) {
    switch (d) {
      case dir::TOP_LEFT: [[fallthrough]];
      case dir::BOTTOM_RIGHT: return DIAGONAL_LR;

      case dir::TOP_RIGHT: [[fallthrough]];
      case dir::BOTTOM_LEFT: return DIAGONAL_RL;

      case dir::TOP: [[fallthrough]];
      case dir::BOTTOM: return VERTICAL;

      case dir::LEFT: [[fallthrough]];
      case dir::RIGHT: return HORIZONTAL;
    }
    assert(false);
    return HORIZONTAL;
  }

  static constexpr bool is_diagonal(dir const d) {
    switch (d) {
      case dir::TOP_LEFT:
      case dir::BOTTOM_RIGHT:
      case dir::TOP_RIGHT: [[fallthrough]];
      case dir::BOTTOM_LEFT: return true;
      default: return false;
    }
    assert(false);
    return false;
  }

  template <typename Fn>
  static constexpr void for_each_opposite(dir const d, Fn&& f) {
    switch (d) {
      case dir::TOP_LEFT:
        f(dir::BOTTOM);
        f(dir::BOTTOM_RIGHT);
        f(dir::RIGHT);
        break;
      case dir::TOP:
        f(dir::BOTTOM_LEFT);
        f(dir::BOTTOM);
        f(dir::BOTTOM_RIGHT);
        break;
      case dir::TOP_RIGHT:
        f(dir::LEFT);
        f(dir::BOTTOM_LEFT);
        f(dir::BOTTOM);
        break;
      case dir::RIGHT:
        f(dir::TOP_LEFT);
        f(dir::LEFT);
        f(dir::BOTTOM_LEFT);
        break;
      case dir::BOTTOM_RIGHT:
        f(dir::LEFT);
        f(dir::TOP_LEFT);
        f(dir::TOP);
        break;
      case dir::BOTTOM:
        f(dir::TOP_LEFT);
        f(dir::TOP);
        f(dir::TOP_RIGHT);
        break;
      case dir::BOTTOM_LEFT:
        f(dir::TOP);
        f(dir::TOP_RIGHT);
        f(dir::RIGHT);
        break;
      case dir::LEFT:
        f(dir::TOP_RIGHT);
        f(dir::RIGHT);
        f(dir::BOTTOM_RIGHT);
        break;
    }
  }

  static constexpr dir get_opposite(dir const d) {
    switch (d) {
      case dir::LEFT: return dir::RIGHT;
      case dir::RIGHT: return dir::LEFT;
      case dir::TOP: return dir::BOTTOM;
      case dir::BOTTOM: return dir::TOP;
      case dir::TOP_LEFT: return dir::BOTTOM_RIGHT;
      case dir::BOTTOM_RIGHT: return dir::TOP_LEFT;
      case dir::BOTTOM_LEFT: return dir::TOP_RIGHT;
      case dir::TOP_RIGHT: return dir::BOTTOM_LEFT;
    }
    assert(false);
    return d;
  }

  explicit ascii_network_parser(std::string_view s)
      : lines_{begin(utl::lines{s}), end(utl::lines{s})} {}

  network parse() {
    for (auto const [y, l] : utl::enumerate(lines_)) {
      if (l.starts_with("#")) {
        curr_station_ =
            net_.stations_.emplace_back(std::make_unique<station>()).get();
        auto mut_line = l;
        mut_line >> curr_station_->id_ >> curr_station_->name_;
        continue;
      }

      for (auto const [x, c] : utl::enumerate(l)) {
        utl::verify(curr_station_ != nullptr, "line {}: station not set", y);
        auto const p = pixel_pos{static_cast<pixel_coord_t>(x),
                                 static_cast<pixel_coord_t>(y)};
        digit_ = 0U;

        switch (c) {
          case HORIZONTAL:
          case VERTICAL:
          case DIAGONAL_LR: [[fallthrough]];
          case DIAGONAL_RL: do_edge(p, static_cast<type>(c)); break;

          case KNOT: do_knot(p); break;

          case LEVEL_JUNCTION_ALT: [[fallthrough]];
          case LEVEL_JUNCTION: do_level_junction(p); break;

          case DIRECTION_TOP:
          case DIRECTION_BOTTOM:
          case DIRECTION_LEFT: [[fallthrough]];
          case DIRECTION_RIGHT:
            do_main_signal(p, static_cast<type>(c), true);
            break;

          case SINGLE_SLIP: [[fallthrough]];
          case SINGLE_SLIP_INVERTED: do_single_slip(p, c); break;

          case END_OF_TRAIN_DETECTOR_L:
            do_directional_node(p, dir::RIGHT, dir::LEFT,
                                node::type::END_OF_TRAIN_DETECTOR, c);
            break;
          case END_OF_TRAIN_DETECTOR_R:
            do_directional_node(p, dir::LEFT, dir::RIGHT,
                                node::type::END_OF_TRAIN_DETECTOR, c);
            break;

          case APPROACH_SIGNAL_L:
            do_directional_node(p, dir::RIGHT, dir::LEFT,
                                node::type::APPROACH_SIGNAL, c);
            break;
          case APPROACH_SIGNAL_R:
            do_directional_node(p, dir::LEFT, dir::RIGHT,
                                node::type::APPROACH_SIGNAL, c);
            break;

          default:
            if (std::isdigit(c) != 0) {
              do_digit(p, static_cast<unsigned>(c - '0'));
            } else if (c >= 'a' && c <= 'z') {
              do_end_node(p, c);
            } else if (std::isalpha(c) != 0) {
              do_main_signal(p, static_cast<type>(c), false);
            }
        }
      }
    }

    connect_level_junctions();
    connect_switches();
    connect_single_slip_switches();
    connect_signals();
    connect_directionals();
    connect_edges();

    return std::move(net_);
  }

  template <typename Fn>
  void for_each_neighbor_pos(pixel_pos const p, Fn&& fn) {
    auto const call = [&](dir const d) {
      if (auto const neighbor_pos = next(p, d);
          is_valid_and_non_empty(neighbor_pos)) {
        fn(neighbor_pos, d);
      }
    };
    call(dir::TOP);
    call(dir::BOTTOM);
    call(dir::LEFT);
    call(dir::RIGHT);
    call(dir::TOP_LEFT);
    call(dir::BOTTOM_RIGHT);
    call(dir::TOP_RIGHT);
    call(dir::BOTTOM_LEFT);
  }

  template <typename T, typename Fn>
  void for_each_neighbor(pixel_pos const p, Fn&& fn) {
    for_each_neighbor_pos(p, [&](pixel_pos const neighbor_pos, dir const d) {
      auto const neighbor = get_map_el(neighbor_pos, get_orientation(d));
      if (neighbor.has_value() && cista::holds_alternative<T*>(*neighbor)) {
        fn(cista::get<T*>(*neighbor), d);
      }
    });
  }

  bool is_valid_and_non_empty(pixel_pos const p) const {
    assert(p.valid());
    return static_cast<size_t>(p.y_) < lines_.size() &&
           static_cast<size_t>(p.x_) < lines_[p.y_].len &&
           lines_[p.y_][0] != '#' && lines_[p.y_][p.x_] != EMPTY;
  }

  std::optional<char> get_char(pixel_pos const p) const {
    return is_valid_and_non_empty(p) ? std::make_optional(lines_[p.y_][p.x_])
                                     : std::nullopt;
  }

  std::optional<map_el_t> get_map_el(pixel_pos const p,
                                     type const orientation) {
    if (auto const it1 = map_.find(p); it1 != end(map_)) {
      if (auto const it2 = it1->second.find(orientation);
          it2 != end(it1->second)) {
        return it2->second;
      }
      if (auto const it2 = it1->second.find(KNOT); it2 != end(it1->second)) {
        return it2->second;
      }
    }
    return std::nullopt;
  }

  void do_digit(pixel_pos const p, unsigned digit) {
    digit_ = digit;
    cr::hash_map<type, edge*> els;
    for_each_neighbor_pos(p, [&](pixel_pos const neighbor_pos, dir const d) {
      auto const neighbor_char = get_char(neighbor_pos);
      if (*neighbor_char == get_orientation(d) ||
          std::isdigit(*neighbor_char) != 0) {
        auto const neighbor = get_map_el(neighbor_pos, get_orientation(d));
        if (auto const it = els.find(get_orientation(d));
            it == end(els) || it->second == nullptr) {
          els[get_orientation(d)] =
              neighbor.has_value() && cista::holds_alternative<edge*>(*neighbor)
                  ? cista::get<edge*>(*neighbor)
                  : nullptr;
        }
      }
    });
    utl::verify(els.size() == 1, "num {} {}x connected", p, els.size());
    auto const [orientation, e] = *els.begin();
    do_edge(p, orientation, map_el_t{e});
  }

  void do_edge(pixel_pos const pos, type const orientation) {
    pixel_pos pred_pos;
    switch (orientation) {
      case HORIZONTAL: pred_pos = next(pos, dir::LEFT); break;
      case VERTICAL: pred_pos = next(pos, dir::TOP); break;
      case DIAGONAL_RL: pred_pos = next(pos, dir::TOP_RIGHT); break;
      case DIAGONAL_LR: pred_pos = next(pos, dir::TOP_LEFT); break;
      default: assert(false);
    }
    auto const el_at_pos = get_map_el(pred_pos, orientation);
    do_edge(pos, orientation,
            el_at_pos.has_value() ? *el_at_pos
                                  : map_el_t{static_cast<edge*>(nullptr)});
  }

  void do_edge(pixel_pos const pos, type const orientation, map_el_t pred) {
    edge* e{cista::holds_alternative<edge*>(pred) ? cista::get<edge*>(pred)
                                                  : nullptr};
    if (e == nullptr) {
      e = net_.edges_.emplace_back(std::make_unique<edge>()).get();
    }
    e->draw_representation_.emplace_back(pos, static_cast<char>(orientation));
    e->dist_ += 1;
    e->id_ = digit_ == 0 ? e->id_ : e->id_ * 10 + digit_;
    map_[pos][orientation] = e;
  }

  bool is_edge(pixel_pos const pos) const {
    auto const c = get_char(pos);
    return c.has_value() && (*c == HORIZONTAL || *c == VERTICAL ||
                             *c == DIAGONAL_RL || *c == DIAGONAL_LR);
  }

  void do_knot(pixel_pos const p) {
    auto neighbor_edges{0U};
    auto built_edge_positions{0U};
    cr::hash_set<edge*> edges;
    for_each_neighbor_pos(p, [&](pixel_pos const pos, dir const d) {
      auto const el = get_map_el(pos, get_orientation(d));
      neighbor_edges += static_cast<unsigned>(is_edge(pos));
      built_edge_positions += static_cast<unsigned>(el.has_value());
      if (el.has_value() && cista::holds_alternative<edge*>(*el)) {
        edges.emplace(cista::get<edge*>(*el));
      }
    });

    if (neighbor_edges < 3U) {
      // ### JUST A CORNER (NOT A SWITCH)
      if (built_edge_positions == 2) {
        // Two predecessors need to be handled specially.
        if (edges.size() == 1) {
          // The same edge from two directions. This is a circle!
          // Let's celebrate this with a new node.
          auto const n =
              net_.nodes_.emplace_back(std::make_unique<node>()).get();
          auto const e = *edges.begin();
          n->traversals_[e].emplace(e);
          n->draw_representation_.emplace_back(p, KNOT);
          e->from_ = e->to_ = n;
        } else /* edges.size() == 2 */ {
          // We have built one edge too many. We need to let one go.
          // Happens with _| forms.
          auto const good = *edges.begin();
          auto const bad = *std::next(edges.begin());
          good->draw_representation_.insert(begin(good->draw_representation_),
                                            begin(bad->draw_representation_),
                                            end(bad->draw_representation_));
          good->dist_ += bad->dist_;
          good->id_ = good->id_ != 0U ? good->id_ : bad->id_;
          utl::erase_if(net_.edges_, [&](auto&& e) { return e.get() == bad; });
          for (auto& [pos, els] : map_) {
            for (auto& [orientation, el] : els) {
              el.apply(utl::overloaded{
                  [&](edge*& e) { e = (e == bad) ? good : e; }});
            }
          }
          edges.erase(std::next(edges.begin()));
        }
      }

      // Connect to existing edge if possible. Else, just build a new one.
      do_edge(p, KNOT,
              edges.empty() ? map_el_t{static_cast<edge*>(nullptr)}
                            : map_el_t{*edges.begin()});
    } else {
      // ### SWITCH
      auto const n = net_.nodes_.emplace_back(std::make_unique<node>()).get();
      n->draw_representation_.emplace_back(p, KNOT);
      n->type_ = node::type::SWITCH;
      map_[p][KNOT] = n;
    }
  }

  void do_level_junction(pixel_pos const p) {
    auto const n = net_.nodes_.emplace_back(std::make_unique<node>()).get();
    n->draw_representation_.emplace_back(p, LEVEL_JUNCTION);
    n->type_ = node::type::LEVEL_JUNCTION;
    map_[p][KNOT] = n;
  }

  void do_single_slip(pixel_pos const p, char const type) {
    auto const n = net_.nodes_.emplace_back(std::make_unique<node>()).get();
    n->draw_representation_.emplace_back(p, static_cast<char>(type));
    n->type_ = node::type::SINGLE_SLIP_SWITCH;
    map_[p][KNOT] = n;
  }

  void do_directional_node(pixel_pos const p, dir const from, dir const to,
                           node::type const type, char const content) {
    auto const n = net_.nodes_.emplace_back(std::make_unique<node>()).get();
    n->type_ = type;
    n->draw_representation_.emplace_back(p, content);
    dir_[n] = {p, from, to};
    map_[p][KNOT] = n;
  }

  void do_main_signal(pixel_pos const p, type representation,
                      bool const is_dir) {
    // Search for existing adjacent part of the signal name.
    node* signal{nullptr};
    type signal_orientation{KNOT};
    for_each_neighbor_pos(p, [&](pixel_pos const neighbor_pos, dir const d) {
      if (auto const it1 = map_.find(neighbor_pos); it1 != end(map_)) {
        if (auto const it2 = it1->second.find(get_orientation(d));
            it2 != end(it1->second)) {
          if (cista::holds_alternative<node*>(it2->second) &&
              cista::get<node*>(it2->second)->type_ ==
                  node::type::MAIN_SIGNAL) {
            signal = cista::get<node*>(it2->second);
            signal_orientation = get_orientation(d);
          } else if (cista::holds_alternative<edge*>(it2->second)) {
            signal_orientation = get_orientation(d);
          }
        }
      }
    });

    utl::verify(signal_orientation != KNOT, "signal {} has no neighbor", p);

    if (signal == nullptr) {
      signal = net_.nodes_.emplace_back(std::make_unique<node>()).get();
      signal->type_ = node::type::MAIN_SIGNAL;
    }
    map_[p][signal_orientation] = map_el_t{signal};

    auto& signal_info = signal_info_[signal];
    signal_info.positions_.emplace_back(p);
    if (is_dir) {
      signal_info.direction_ = representation;
    } else {
      signal->name_ += static_cast<char>(representation);
    }
    signal->draw_representation_.emplace_back(
        p, static_cast<char>(representation));
  }

  void do_end_node(pixel_pos const p, char c) {
    auto const n = net_.nodes_.emplace_back(std::make_unique<node>()).get();
    n->type_ = node::type::END_NODE;
    n->draw_representation_.emplace_back(p, c);
    n->name_ = c;
    map_[p][KNOT] = map_el_t{n};
  }

  void connect_switches() {
    for (auto const& [p, el] : map_) {
      auto const pos = p;

      if (el.size() != 1U ||
          !cista::holds_alternative<node*>(begin(el)->second)) {
        continue;
      }

      auto const n = cista::get<node*>(begin(el)->second);
      if (n->type_ != node::type::SWITCH) {
        continue;
      }

      for_each_neighbor<edge>(pos, [&](edge* e, dir const d) {
        for_each_opposite(d, [&](dir const opposite_dir) {
          if (auto const opposite_el = get_map_el(
                  next(pos, opposite_dir), get_orientation(opposite_dir));
              opposite_el.has_value() &&
              cista::holds_alternative<edge*>(*opposite_el)) {
            auto const opposite_e = cista::get<edge*>(*opposite_el);
            n->traversals_[e].emplace(opposite_e);
            n->traversals_[opposite_e].emplace(e);
          }
        });
      });
    }
  }

  std::pair<edge*, type> get_edge(pixel_pos const p,
                                  std::initializer_list<dir> directions) {
    edge* e{nullptr};
    type orientation{KNOT};
    for (auto const dir : directions) {
      auto const el = get_map_el(next(p, dir), get_orientation(dir));
      if (!el.has_value()) {
        continue;
      }
      utl::verify(cista::holds_alternative<edge*>(*el),
                  "single slip neighbor {} not an edge", next(p, dir));
      utl::verify(e == nullptr, "double edge");
      e = cista::get<edge*>(*el);
      orientation = get_orientation(dir);
    }
    utl::verify(e != nullptr, "single slip {} edge missing {}", p,
                *directions.begin());
    return std::pair{e, orientation};
  }

  void connect_single_slip_switches() {
    for (auto const& [p, el] : map_) {
      auto const pos = p;

      if (el.size() != 1U ||
          !cista::holds_alternative<node*>(begin(el)->second)) {
        continue;
      }

      auto const n = cista::get<node*>(begin(el)->second);
      if (n->type_ != node::type::SINGLE_SLIP_SWITCH) {
        continue;
      }

      auto const [left, left_dir] = get_edge(pos, {dir::LEFT});
      auto const [right, right_dir] = get_edge(pos, {dir::RIGHT});
      auto const [bottom, bottom_dir] =
          get_edge(pos, {dir::BOTTOM_LEFT, dir::BOTTOM, dir::BOTTOM_RIGHT});
      auto const [top, top_dir] =
          get_edge(pos, {dir::TOP_LEFT, dir::TOP, dir::TOP_RIGHT});

      utl::verify(bottom_dir == top_dir,
                  "single slip {} bottom/top orientation mismatch", p);

      n->traversals_[left].emplace(right);
      n->traversals_[right].emplace(left);
      n->traversals_[top].emplace(bottom);
      n->traversals_[bottom].emplace(top);

      if (n->draw_representation_[0].content_ == SINGLE_SLIP) {
        if (top_dir == DIAGONAL_RL) {
          n->traversals_[left].emplace(top);
          n->traversals_[top].emplace(left);
        } else if (top_dir == DIAGONAL_LR) {
          n->traversals_[right].emplace(top);
          n->traversals_[top].emplace(right);
        }
      } else {
        if (top_dir == DIAGONAL_RL) {
          n->traversals_[right].emplace(top);
          n->traversals_[bottom].emplace(right);
        } else if (top_dir == DIAGONAL_LR) {
          n->traversals_[left].emplace(bottom);
          n->traversals_[bottom].emplace(left);
        }
      }
    }
  }

  void connect_level_junctions() {
    for (auto const& [p, el] : map_) {
      auto const pos = p;

      if (el.size() != 1U ||
          !cista::holds_alternative<node*>(begin(el)->second)) {
        continue;
      }

      auto const n = cista::get<node*>(begin(el)->second);
      if (n->type_ != node::type::LEVEL_JUNCTION) {
        continue;
      }

      for_each_neighbor<edge>(p, [&](edge* e, dir const d) {
        auto const opposite_e =
            get_edge(next(pos, get_opposite(d)), get_orientation(d));
        n->traversals_[e].emplace(opposite_e);
        n->traversals_[opposite_e].emplace(e);
      });
    }
  }

  void connect_signals() {
    for (auto const& [signal, info] : signal_info_) {
      pixel_pos pos;
      cr::hash_map<dir, edge*> neighbors;
      for (auto const& p : info.positions_) {
        pos = p;
        for_each_neighbor<edge>(
            p, [&](edge* e, dir const d) { neighbors.emplace(d, e); });
      }

      edge *from{nullptr}, *to{nullptr};
      switch (info.direction_) {
        case DIRECTION_RIGHT:
          from = neighbors.at(dir::LEFT);
          to = neighbors.at(dir::RIGHT);
          break;

        case DIRECTION_LEFT:
          from = neighbors.at(dir::RIGHT);
          to = neighbors.at(dir::LEFT);
          break;

        case DIRECTION_TOP:
          from = neighbors.at(dir::BOTTOM);
          to = neighbors.at(dir::TOP);
          break;

        case DIRECTION_BOTTOM:
          from = neighbors.at(dir::TOP);
          to = neighbors.at(dir::BOTTOM);
          break;

        default: utl::verify(false, "signal {}: bad direction", pos);
      }

      utl::verify(from != nullptr, "signal {} from not found", pos);
      utl::verify(to != nullptr, "signal {} to not found", pos);

      signal->action_traversals_[from] = to;
      signal->traversals_[from].emplace(to);
      signal->traversals_[to].emplace(from);
    }
  }

  edge* get_edge(pixel_pos const p, type const orientation) {
    auto const e = get_map_el(p, orientation);
    utl::verify(e.has_value(), "{} is empty but should be an edge");
    utl::verify(cista::holds_alternative<edge*>(*e), "{} is not an edge", p);
    return cista::get<edge*>(*e);
  }

  void connect_directionals() {
    for (auto const& [n, info] : dir_) {
      auto const [pos, from, to] = info;
      auto const from_edge = get_edge(next(pos, from), get_orientation(from));
      auto const to_edge = get_edge(next(pos, to), get_orientation(to));
      n->traversals_[from_edge].emplace(to_edge);
      n->traversals_[to_edge].emplace(from_edge);
      n->action_traversals_[from_edge] = to_edge;
    }
  }

  void connect_edges() {
    for (auto const& [pos, els] : map_) {
      for (auto const& [orientation, el] : els) {
        if (!cista::holds_alternative<node*>(el)) {
          continue;
        }
        auto const n = cista::get<node*>(el);
        for_each_neighbor<edge>(pos, [&](edge* e, dir const) {
          e->add_node(n);
          if (n->type_ == node::type::END_NODE) {
            n->end_node_edge_ = e;
          }
        });
      }
    }
  }

  std::vector<utl::cstr> lines_;
  cr::hash_map<pixel_pos, cr::hash_map<type /* orientation */, map_el_t>> map_;
  cr::hash_map<node*, signal_info> signal_info_;
  cr::hash_map<node*, std::tuple<pixel_pos, dir, dir>> dir_;
  unsigned digit_{0U};
  station* curr_station_{nullptr};
  network net_;
};

network parse_network(std::string_view str) {
  return ascii_network_parser{str}.parse();
}

}  // namespace rapid