#include "soro/infrastructure/graph/graph_creation.h"

#include "utl/verify.h"

#include "soro/utls/string.h"

namespace soro::infra {

using namespace utl;
using namespace soro::si;
using namespace soro::utls;

element* create_element(graph& network, station& station,
                        construction_materials& mats, type const type,
                        rail_plan_node_id const rp_id, bool const rising) {
  if (is_cross(type)) {
    return create_element_t<cross>(network, station, mats, type, rp_id, rising);
  } else if (is_end_element(type)) {
    return create_element_t<end_element>(network, station, mats, type, rp_id,
                                         rising);
  } else if (is_simple_element(type)) {
    return create_element_t<simple_element>(network, station, mats, type, rp_id,
                                            rising);
  } else if (is_simple_switch(type)) {
    return create_element_t<simple_switch>(network, station, mats, type, rp_id,
                                           rising);
  } else if (is_directed_track_element(type)) {
    return create_element_t<track_element>(network, station, mats, type, rp_id,
                                           rising);
  } else if (is_undirected_track_element(type)) {
    return create_element_t<undirected_track_element>(network, station, mats, type, rp_id,
                                           rising);
  }

  throw utl::fail("Could not dispatch create element for type: {}", type);
}

element* get_or_create_element(graph& network, station& station,
                               construction_materials& mats, type const type,
                               rail_plan_node_id const rp_id,
                               bool const rising) {
  auto element_it = mats.rp_id_to_element_id_.find(rp_id);
  if (element_it == std::end(mats.rp_id_to_element_id_)) {
    return create_element(network, station, mats, type, rp_id, rising);
  } else {
    return network.element_store_[element_it->second].get();
  }
};

void set_km_point_and_line(end_element& e, std::string const&,
                           kilometrage const km_point, line_id const line) {
  e.km_ = km_point;
  e.line_ = line;
}

void set_km_point_and_line(simple_element& e, std::string const& node_name,
                           kilometrage const km_point, line_id const line) {
  switch (str_hash(node_name)) {
    case str_hash(KM_JUMP_START):
    case str_hash(LINE_SWITCH_ZERO): {
      e.start_km_ = km_point;
      e.start_line_ = line;
      return;
    }

    case str_hash(KM_JUMP_END):
    case str_hash(LINE_SWITCH_ONE): {
      e.end_km_ = km_point;
      e.end_line_ = line;
      return;
    }

    case str_hash(BORDER): {
      e.start_km_ = km_point;
      e.end_km_ = km_point;
      e.start_line_ = line;
      e.end_line_ = line;
      return;
    }
  }

  throw utl::fail("Unknown node name in set kilometre point in simple element");
}

void set_km_point_and_line(track_element& e, std::string const&,
                           kilometrage const km_point, line_id const line) {
  e.km_ = km_point;
  e.line_ = line;
}

void set_km_point_and_line(undirected_track_element& e, std::string const&,
                           kilometrage const km_point, line_id const line) {
  e.km_ = km_point;
  e.line_ = line;
}

void set_km_point_and_line(simple_switch& e, std::string const& node_name,
                           kilometrage const km_point, line_id const line) {
  switch (str_hash(node_name)) {
    case str_hash(SWITCH_START): {
      e.start_km_ = km_point;
      e.start_line_ = line;
      return;
    }
    case str_hash(SWITCH_STEM): {
      e.stem_km_ = km_point;
      e.stem_line_ = line;
      return;
    }
    case str_hash(SWITCH_BRANCH_LEFT):
    case str_hash(SWITCH_BRANCH_RIGHT): {
      e.branch_km_ = km_point;
      e.branch_line_ = line;
      return;
    }
  }

  throw utl::fail("Unknown node name in set kilometre point in simple switch");
}

void set_km_point_and_line(cross& e, std::string const& node_name,
                           kilometrage const km_point, line_id const line) {
  switch (str_hash(node_name)) {
    case str_hash(CROSS_SWITCH_START_LEFT):
    case str_hash(CROSS_START_LEFT): {
      e.start_left_km_ = km_point;
      e.start_left_line_ = line;
      return;
    }
    case str_hash(CROSS_SWITCH_END_LEFT):
    case str_hash(CROSS_END_LEFT): {
      e.end_left_km_ = km_point;
      e.end_left_line_ = line;
      return;
    }
    case str_hash(CROSS_SWITCH_START_RIGHT):
    case str_hash(CROSS_START_RIGHT): {
      e.start_right_km_ = km_point;
      e.start_right_line_ = line;
      return;
    }
    case str_hash(CROSS_SWITCH_END_RIGHT):
    case str_hash(CROSS_END_RIGHT): {
      e.end_right_km_ = km_point;
      e.end_right_line_ = line;
      return;
    }
  }
  throw utl::fail("Unknown node name in set kilometre point in cross");
}

void set_km_point_and_line(element& e, std::string const& node_name,
                           kilometrage km_point, line_id line) {
  e.apply(
      [&](auto&& x) { set_km_point_and_line(x, node_name, km_point, line); });
}

void set_neighbour(end_element& e, std::string const&, element* neigh,
                   bool rising) {
  (rising ? e.rising_neighbour() : e.falling_neighbour()) = neigh;
}

void set_neighbour(simple_element& e, std::string const& name, element* neigh,
                   bool rising) {
  switch (str_hash(name)) {
    case str_hash(KM_JUMP_START):
    case str_hash(LINE_SWITCH_ZERO):
      (rising ? e.start_rising_neighbour() : e.start_falling_neighbour()) =
          neigh;
      return;

    case str_hash(KM_JUMP_END):
    case str_hash(LINE_SWITCH_ONE):
      (rising ? e.end_rising_neighbour() : e.end_falling_neighbour()) = neigh;
      return;

    default: {
      if (e.rising_) {
        (rising ? e.end_rising_neighbour() : e.end_falling_neighbour()) = neigh;
      } else {
        (rising ? e.start_rising_neighbour() : e.start_falling_neighbour()) =
            neigh;
      }
    }
  }
}

void set_neighbour(simple_switch& e, std::string const& name, element* neigh,
                   bool rising) {
  switch (str_hash(name)) {
    case str_hash(SWITCH_START):
      (rising ? e.rising_start_neighbour() : e.falling_start_neighbour()) =
          neigh;
      return;
    case str_hash(SWITCH_STEM):
      (rising ? e.rising_stem_neighbour() : e.falling_stem_neighbour()) = neigh;
      return;
    case str_hash(SWITCH_BRANCH_LEFT):
    case str_hash(SWITCH_BRANCH_RIGHT):
      (rising ? e.rising_branch_neighbour() : e.falling_branch_neighbour()) =
          neigh;
      return;
  }
}

void set_neighbour(track_element& e, std::string const&, element* neigh,
                   bool rising) {
  (rising ? e.ahead() : e.behind()) = neigh;
}

void set_neighbour(undirected_track_element& e, std::string const&,
                   element* neigh, bool rising) {
  if (rising) {
    e.end_rising_neighbour() == nullptr ? e.end_rising_neighbour() = neigh
                                          : e.start_rising_neighbour() = neigh;
  } else {
    e.end_falling_neighbour() == nullptr ? e.end_falling_neighbour() = neigh
                                           : e.start_falling_neighbour() = neigh;
  }
}

void set_neighbour(cross& e, std::string const& name, element* neigh,
                   bool rising) {
  switch (str_hash(name)) {
    case str_hash(CROSS_START_LEFT):
    case str_hash(CROSS_SWITCH_START_LEFT): {
      (rising ? e.rising_start_left() : e.falling_start_left()) = neigh;
      return;
    }
    case str_hash(CROSS_END_LEFT):
    case str_hash(CROSS_SWITCH_END_LEFT): {
      (rising ? e.rising_end_left() : e.falling_end_left()) = neigh;
      return;
    }
    case str_hash(CROSS_START_RIGHT):
    case str_hash(CROSS_SWITCH_START_RIGHT): {
      (rising ? e.rising_start_right() : e.falling_start_right()) = neigh;
      return;
    }
    case str_hash(CROSS_END_RIGHT):
    case str_hash(CROSS_SWITCH_END_RIGHT): {
      (rising ? e.rising_end_right() : e.falling_end_right()) = neigh;
      return;
    }
  }
}

void set_neighbour(element& e, std::string const& name, element* neigh,
                   bool const rising) {
  e.apply([&](auto&& x) { set_neighbour(x, name, neigh, rising); });
}

bool joins_lines(end_element const&) { return false; }
bool joins_lines(track_element const&) { return false; }
bool joins_lines(undirected_track_element const&) { return false; }
bool joins_lines(simple_element const&) { return false; }

bool joins_lines(simple_switch const& s) {
  return !(s.start_line_ == s.stem_line_ && s.start_line_ == s.branch_line_);
}

bool joins_lines(cross const& c) {
  return !(c.start_left_line_ == c.end_left_line_ &&
           c.start_left_line_ == c.start_right_line_ &&
           c.start_right_line_ == c.end_right_line_);
}

bool misaligned_join(end_element const&) { return false; }
bool misaligned_join(track_element const&) { return false; }
bool misaligned_join(undirected_track_element const&) { return false; }
bool misaligned_join(simple_element const&) { return false; }

bool misaligned_join(simple_switch const& s) {
  if (!joins_lines(s)) {
    return false;
  }

  auto const join_count =
      (static_cast<int>(s.rising_) + static_cast<int>(s.stem_rising_) +
       static_cast<int>(s.branch_rising_));

  return s.rising_ ? join_count != 1 : join_count != 2;
}

bool misaligned_join(cross const& c) {
  return joins_lines(c) &&
         ((static_cast<int>(c.rising_) + static_cast<int>(c.end_left_rising_) +
           static_cast<int>(c.start_right_rising_) +
           static_cast<int>(c.end_right_rising_)) != 2 ||
          c.start_right_rising_ == c.end_right_rising_);
}

bool misaligned_join(element const& e) {
  return e.apply([](auto&& x) { return misaligned_join(x); });
}

node_ptr get_node(end_element const& e, bool const rising, element_ptr) {
  return rising ? e.top() : e.bot();
}

node_ptr get_node(simple_element const& s, bool const rising,
                  element_ptr neigh) {
  if (s.type_ == type::LINE_SWITCH && (neigh == s.end_rising_neighbour() ||
                                       neigh == s.end_falling_neighbour())) {
    bool const swap = s.rising_ == s.end_rising_;
    return swap ? (s.end_rising_ ? s.bot() : s.top())
                : (s.end_rising_ ? s.top() : s.bot());
  }

  return rising ? s.top() : s.bot();
}

node_ptr get_node(track_element const& e, bool const, element_ptr) {
  return e.get_node();
}

node_ptr get_node(undirected_track_element const& e, bool const rising,
                  element_ptr) {
  return rising ? e.top() : e.bot();
}

node_ptr get_node(simple_switch const& e, bool const rising,
                  element_ptr neigh) {
  bool const is_stem =
      neigh == e.rising_stem_neighbour() || neigh == e.falling_stem_neighbour();
  bool const is_branch = neigh == e.rising_branch_neighbour() ||
                         neigh == e.falling_branch_neighbour();

  bool const stem_misalign =
      e.stem_line_ != e.start_line_ && e.rising_ == e.stem_rising_;
  bool const branch_misalign =
      e.branch_line_ != e.start_line_ && e.rising_ == e.branch_rising_;

  bool const swap =
      joins_lines(e) && misaligned_join(e) &&
      ((is_stem && stem_misalign) || (is_branch && branch_misalign));

  return swap ? (rising ? e.bot() : e.top()) : (rising ? e.top() : e.bot());
}

node_ptr get_node(cross const& e, bool const rising, element_ptr neigh) {
  bool const is_start_left =
      neigh == e.rising_start_left() || neigh == e.falling_start_left();
  bool const is_end_left =
      neigh == e.rising_end_left() || neigh == e.falling_end_left();
  bool const is_end_right =
      neigh == e.rising_end_right() || neigh == e.falling_end_right();
  bool const is_start_right =
      neigh == e.rising_start_right() || neigh == e.falling_start_right();

  auto const mis_join = joins_lines(e) && misaligned_join(e);

  if (is_start_left) {
    return rising ? e.left() : e.right();
  } else if (is_end_left) {
    auto const left_misalign = e.end_left_line_ != e.start_left_line_ &&
                               e.rising_ == e.end_left_rising_;
    auto const swap = mis_join && left_misalign;
    return swap ? (e.end_left_rising_ ? e.right() : e.left())
                : (e.end_left_rising_ ? e.left() : e.right());
  } else if (is_start_right) {
    return rising ? e.top() : e.bot();
  } else if (is_end_right) {
    bool const right_misalign = e.end_right_line_ != e.start_right_line_ &&
                                e.start_right_rising_ == e.end_right_rising_;
    auto const swap = mis_join && right_misalign;
    return swap ? (e.end_right_rising_ ? e.bot() : e.top())
                : (e.end_right_rising_ ? e.top() : e.bot());
  }

  throw utl::fail("neigh does not appear in neighbours");
}

node_ptr get_node(element const& e, bool const rising, element_ptr neigh) {
  return e.apply([&](auto&& x) { return get_node(x, rising, neigh); });
}

non_const_ptr<node> to_non_const(node_ptr ptr) {
#if defined(SERIALIZE)
  return static_cast<non_const_ptr<node>>(ptr);
#else
  return const_cast<non_const_ptr<node>>(ptr);  // NOLINT
#endif
}

non_const_element_ptr to_non_const(element_ptr ptr) {
#if defined(SERIALIZE)
  return static_cast<non_const_element_ptr>(ptr);
#else
  return const_cast<non_const_element_ptr>(ptr);  // NOLINT
#endif
}

void connect_nodes(end_element& e, element_ptr this_ptr) {
  auto top_ptr = to_non_const(e.top());
  auto bot_ptr = to_non_const(e.bot());

  if (!e.rising_) {
    top_ptr->next_node_ = get_node(*e.rising_neighbour(), !e.rising_, this_ptr);
  } else {
    bot_ptr->next_node_ =
        get_node(*e.falling_neighbour(), !e.rising_, this_ptr);
  }
}

void connect_nodes(simple_element& e, element_ptr this_ptr) {
  auto top_ptr = to_non_const(e.top());
  auto bot_ptr = to_non_const(e.bot());

  if (e.type_ == type::LINE_SWITCH) {
    if (e.rising_) {

      top_ptr->next_node_ =
          get_node(*(e.end_rising_ ? e.end_falling_neighbour()
                                   : e.end_rising_neighbour()),
                   !e.end_rising_, this_ptr);

      bot_ptr->next_node_ =
          get_node(*e.start_falling_neighbour(), false, this_ptr);
    } else {

      top_ptr->next_node_ =
          get_node(*e.start_rising_neighbour(), true, this_ptr);

      bot_ptr->next_node_ =
          get_node(*(e.end_rising_ ? e.end_falling_neighbour()
                                   : e.end_rising_neighbour()),
                   !e.end_rising_, this_ptr);
    }
  } else {
    if (e.start_rising_neighbour() != nullptr) {
      top_ptr->next_node_ =
          get_node(*e.start_rising_neighbour(), true, this_ptr);
    }
    if (e.end_falling_neighbour() != nullptr) {
      bot_ptr->next_node_ =
          get_node(*e.end_falling_neighbour(), false, this_ptr);
    }
  }
}

void connect_nodes(undirected_track_element& e, element_ptr this_ptr) {
  auto top_ptr = to_non_const(e.top());
  auto bot_ptr = to_non_const(e.bot());

  if (e.start_rising_neighbour() != nullptr) {
    top_ptr->next_node_ = get_node(*e.start_rising_neighbour(), true, this_ptr);
  }

  if (e.end_falling_neighbour() != nullptr) {
    bot_ptr->next_node_ = get_node(*e.end_falling_neighbour(), false, this_ptr);
  }
}

void connect_nodes(simple_switch& e, element_ptr this_ptr) {
  auto const& connect = [&](std::pair<element_ptr, element_ptr> neighs,
                            auto const rising) {
    return get_node(*(rising ? neighs.second : neighs.first), !rising,
                    this_ptr);
  };

  auto top = to_non_const(e.top());
  auto bot = to_non_const(e.bot());

  // is start -> stem in rising kilometer point direction or in falling
  if (e.rising_) {
    bot->next_node_ = connect(
        {e.rising_start_neighbour(), e.falling_start_neighbour()}, e.rising_);
    top->next_node_ =
        connect({e.rising_stem_neighbour(), e.falling_stem_neighbour()},
                e.stem_rising_);
    top->branch_node_ =
        connect({e.rising_branch_neighbour(), e.falling_branch_neighbour()},
                e.branch_rising_);
  } else {
    top->next_node_ = connect(
        {e.rising_start_neighbour(), e.falling_start_neighbour()}, e.rising_);
    bot->next_node_ =
        connect({e.rising_stem_neighbour(), e.falling_stem_neighbour()},
                e.stem_rising_);
    bot->branch_node_ =
        connect({e.rising_branch_neighbour(), e.falling_branch_neighbour()},
                e.branch_rising_);
  }
}

void connect_nodes(track_element& e, element_ptr this_ptr) {
  auto node = to_non_const(e.get_node());
  node->next_node_ = get_node(*e.ahead(), e.rising_, this_ptr);
}

void connect_nodes(cross& e, element_ptr this_ptr) {
  auto const& connect = [&](std::pair<element_ptr, element_ptr> neighs,
                            auto const rising) {
    return get_node(*(rising ? neighs.second : neighs.first), !rising,
                    this_ptr);
  };

  auto top = to_non_const(e.top());
  auto bot = to_non_const(e.bot());
  auto left = to_non_const(e.left());
  auto right = to_non_const(e.right());

  if (e.rising_) {
    right->next_node_ =
        connect({e.rising_start_left(), e.falling_start_left()}, e.rising_);
    left->next_node_ = connect({e.rising_end_left(), e.falling_end_left()},
                               e.end_left_rising_);
  } else {
    left->next_node_ =
        connect({e.rising_start_left(), e.falling_start_left()}, e.rising_);
    right->next_node_ = connect({e.rising_end_left(), e.falling_end_left()},
                                e.end_left_rising_);
  }

  if (e.start_right_rising_) {
    bot->next_node_ = connect({e.rising_start_right(), e.falling_start_right()},
                              e.start_right_rising_);
    top->next_node_ = connect({e.rising_end_right(), e.falling_end_right()},
                              e.end_right_rising_);
  } else {
    top->next_node_ = connect({e.rising_start_right(), e.falling_start_right()},
                              e.start_right_rising_);
    bot->next_node_ = connect({e.rising_end_right(), e.falling_end_right()},
                              e.end_right_rising_);
  }

  if (e.start_left_end_right_arc_) {
    (e.rising_ ? left : right)->branch_node_ = connect(
        {e.rising_end_right(), e.falling_end_right()}, e.end_right_rising_);

    (e.start_right_rising_ ? bot : top)->branch_node_ =
        connect({e.rising_start_left(), e.falling_start_left()}, e.rising_);
  }

  if (e.start_right_end_left_arc_) {
    (e.start_right_rising_ ? top : bot)->branch_node_ = connect(
        {e.rising_end_left(), e.falling_end_left()}, e.end_left_rising_);

    (e.rising_ ? right : left)->branch_node_ =
        connect({e.rising_start_right(), e.falling_start_right()},
                e.start_right_rising_);
  }
}

void connect_nodes(element& e) {
  return e.apply([&](auto&& x) { return connect_nodes(x, &e); });
}

void connect_nodes(graph& n) {
  for (auto& element : n.elements_) {
    auto non_const_element = to_non_const(element);
    connect_nodes(*non_const_element);
  }

  for (auto& node : n.nodes_) {
    if (node->next_node_ != nullptr) {
      auto next_node = to_non_const(node->next_node_);
      next_node->reverse_edges_.emplace_back(node);
    }

    if (node->branch_node_ != nullptr) {
      auto branch_node = to_non_const(node->branch_node_);
      branch_node->reverse_edges_.emplace_back(node);
    }
  }
}

section::id create_section(graph& n) {
  n.sections_.resize(n.sections_.size() + 1);
  return static_cast<section::id>(n.sections_.size() - 1);
}

void connect_border(simple_element& from_border, bool low_border,
                    element_ptr to_border) {
  assert(to_border != nullptr);
  if (low_border) {
    from_border.end_rising_neighbour() = to_border;
    from_border.end_falling_neighbour() = to_border;
  } else {
    from_border.start_rising_neighbour() = to_border;
    from_border.start_falling_neighbour() = to_border;
  }
}

}  // namespace soro::infra
