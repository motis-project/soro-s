#include "soro/infrastructure/graph/construction/connect_nodes.h"

#include "utl/verify.h"

#include "soro/base/soro_types.h"

#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/all_of.h"
#include "soro/utls/std_wrapper/contains.h"

#include "soro/infrastructure/graph/construction/joins_lines.h"
#include "soro/infrastructure/graph/construction/set_neighbour.h"
#include "soro/infrastructure/graph/construction/starts_section.h"
#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/graph.h"
#include "soro/infrastructure/graph/section.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/kilometrage.h"
#include "soro/infrastructure/station/station.h"

namespace soro::infra {

using namespace utls;

node::ptr get_connecting_node(end_element const& e, mileage_dir const dir,
                              element::ptr /* neigh */, graph const& /* g */) {
  using enum end_element::nodes;
  return e.node(is_rising(dir) ? top : bot);
}

node::ptr get_connecting_node(simple_element const& s, mileage_dir const dir,
                              element::ptr const neigh, graph const& g) {
  using enum mileage_dir;
  using enum simple_element::nodes;
  using enum simple_element::direction;

  if (s.type_ == type::LINE_SWITCH && contains(s.neighbours(second), neigh)) {
    auto const first_pos = g.sections_.get_section_position(s.id_, first);
    auto const second_pos = g.sections_.get_section_position(s.id_, second);

    bool const swap = first_pos == second_pos;

    return swap ? (is_rising(dir) ? s.node(bot) : s.node(top))
                : (is_rising(dir) ? s.node(top) : s.node(bot));
  }

  return s.node(is_rising(dir) ? top : bot);
}

node::ptr get_connecting_node(track_element const& e,
                              mileage_dir const /* dir */,
                              element::ptr const /* neigh */,
                              graph const& /* g */) {
  return e.node(track_element::nodes::one);
}

node::ptr get_connecting_node(simple_switch const& e, mileage_dir const dir,
                              element::ptr const neigh, graph const& g) {
  using enum mileage_dir;
  using enum simple_switch::nodes;
  using enum simple_switch::direction;

  auto const is_stem = contains(e.neighbours(stem), neigh);
  auto const is_branch = contains(e.neighbours(branch), neigh);

  auto const start_pos = g.sections_.get_section_position(e.id_, start);
  auto const stem_pos = g.sections_.get_section_position(e.id_, stem);
  auto const branch_pos = g.sections_.get_section_position(e.id_, branch);

  bool const stem_misalign =
      e.get_line(stem) != e.get_line(start) && start_pos == stem_pos;
  bool const branch_misalign =
      e.get_line(branch) != e.get_line(start) && start_pos == branch_pos;

  bool const swap =
      joins_lines(e) && misaligned_join(e, g) &&
      ((is_stem && stem_misalign) || (is_branch && branch_misalign));

  return swap ? e.node(is_rising(dir) ? bot : top)
              : e.node(is_rising(dir) ? top : bot);
}

bool right_misalign(cross const& c, graph const& g) {
  using enum cross::nodes;
  using enum cross::direction;

  auto const mis_join = misaligned_join(c, g);

  auto const sr_pos = g.sections_.get_section_position(c.id_, start_right);
  auto const er_pos = g.sections_.get_section_position(c.id_, end_right);

  bool const right_misalign =
      c.get_line(end_right) != c.get_line(start_right) && sr_pos == er_pos;

  return mis_join && right_misalign;
}

bool left_misalign(cross const& c, graph const& g) {
  using enum cross::nodes;
  using enum cross::direction;

  auto const mis_join = misaligned_join(c, g);

  auto const sl_pos = g.sections_.get_section_position(c.id_, start_left);
  auto const el_pos = g.sections_.get_section_position(c.id_, end_left);

  auto const left_misalign =
      c.get_line(start_left) != c.get_line(end_left) && sl_pos == el_pos;

  return mis_join && left_misalign;
}

node::ptr get_connecting_node(cross const& e, mileage_dir const,
                              element::ptr const neigh, graph const&) {
  using enum mileage_dir;
  using enum cross::nodes;
  using enum cross::direction;

  // identify which neighbour we are dealing with
  auto const is_start_left = contains(e.neighbours(start_left), neigh);
  auto const is_end_left = contains(e.neighbours(end_left), neigh);
  auto const is_start_right = contains(e.neighbours(start_right), neigh);
  auto const is_end_right = contains(e.neighbours(end_right), neigh);

  //  auto const sl_pos = g.sections_.get_section_position(e.id_, start_left);
  //  auto const sr_pos = g.sections_.get_section_position(e.id_, start_right);

  if (is_start_left)
    return e.node(sl_to_el);
  else if (is_end_left)
    return e.node(el_to_sl);
  else if (is_start_right)
    return e.node(sr_to_er);
  else if (is_end_right)
    return e.node(er_to_sr);

  //  if (is_end_left) {
  //    auto const swap = left_misalign(e, g);
  //
  //    return e.node((is_rising(dir) && !swap) || (!is_rising(dir) && swap)
  //                      ? left_rising
  //                      : left_falling);
  //
  //    if ((is_rising(dir) && !swap) || (!is_rising(dir) && swap)) {
  //      return e.node(left_rising);
  //    } else if ((!is_rising(dir) && !swap) || (is_rising(dir) && swap)) {
  //      return e.node(left_falling);
  //    }
  //  }
  //
  //  if (is_start_right) {
  //    return e.node(is_rising(dir) ? right_rising : right_falling);
  //  }
  //
  //  if (is_end_right) {
  //    auto const swap = right_misalign(e, g);
  //
  //    return swap
  //               ? (is_rising(dir) ? e.node(right_falling) :
  //               e.node(right_rising)) : (is_rising(dir) ?
  //               e.node(right_rising)
  //                                 : e.node(right_falling));
  //  }

  throw utl::fail("neigh does not appear in neighbours");
}

node::ptr get_connecting_node(element::ptr const e, mileage_dir const dir,
                              element::ptr const neigh, graph const& g) {
  return e->apply(
      [&](auto&& x) { return get_connecting_node(x, dir, neigh, g); });
}

// accessor helper to get a non const pointer to a node,
// as we are setting the next/branch pointer of every node here
template <typename Nodes>
node* get_node(element::ptr const e, Nodes const n, graph& g) {
  auto const node_id = e->node(n)->id_;
  return g.node_store_[as_val(node_id)].get();
}

void connect_nodes(end_element const& ee, element::ptr const e, graph& g) {
  using enum mileage_dir;
  using enum end_element::nodes;
  using enum end_element::direction;

  auto const section_pos = g.sections_.get_section_position(ee.id_, oneway);
  auto const dir = section_pos_to_direction(section_pos);

  auto from = get_node(e, is_start(section_pos) ? top : bot, g);
  auto to = get_connecting_node(ee.neighbour(dir, oneway), dir, e, g);

  from->next_ = to;
}

void connect_nodes(simple_element const& se, element::ptr const e, graph& g) {
  using enum mileage_dir;
  using enum simple_element::nodes;
  using enum simple_element::direction;

  auto top_node = get_node(e, top, g);
  auto bot_node = get_node(e, bot, g);

  auto const first_pos = g.sections_.get_section_position(se.id_, first);
  auto const second_pos = g.sections_.get_section_position(se.id_, second);

  if (e->type() == type::LINE_SWITCH) {
    auto f1 = is_start(first_pos) ? top_node : bot_node;
    auto f2 = is_start(first_pos) ? bot_node : top_node;

    auto const first_dir = section_pos_to_direction(first_pos);
    auto const second_dir = section_pos_to_direction(second_pos);

    f1->next_ =
        get_connecting_node(se.neighbour(first_dir, first), first_dir, e, g);
    f2->next_ =
        get_connecting_node(se.neighbour(second_dir, second), second_dir, e, g);

  } else {
    utls::sassert(first_pos != second_pos, "non line switches must align");

    auto const top_side = is_start(first_pos) ? first : second;
    auto const bot_side = is_start(second_pos) ? first : second;

    top_node->next_ =
        get_connecting_node(se.neighbour(rising, top_side), rising, e, g);
    bot_node->next_ =
        get_connecting_node(se.neighbour(falling, bot_side), falling, e, g);
  }
}

void connect_nodes(simple_switch const& ss, element::ptr const e, graph& g) {
  using enum simple_switch::nodes;
  using enum simple_switch::direction;

  auto const start_pos = g.sections_.get_section_position(ss.id_, start);
  auto const stem_pos = g.sections_.get_section_position(ss.id_, stem);
  auto const branch_pos = g.sections_.get_section_position(ss.id_, branch);

  auto top_node = get_node(e, top, g);
  auto bot_node = get_node(e, bot, g);

  // we have to create three outgoing edges
  auto from_start = is_start(start_pos) ? top_node : bot_node;
  auto from_stem = is_start(start_pos) ? bot_node : top_node;
  auto from_branch = is_start(start_pos) ? bot_node : top_node;

  auto const start_dir = section_pos_to_direction(start_pos);
  auto const stem_dir = section_pos_to_direction(stem_pos);
  auto const branch_dir = section_pos_to_direction(branch_pos);

  utls::sassert(from_start != from_stem, "no overwriting");
  utls::sassert(from_stem == from_branch, "stem and branch must be the same");

  from_start->next_ =
      get_connecting_node(ss.neighbour(start_dir, start), start_dir, e, g);
  from_stem->next_ =
      get_connecting_node(ss.neighbour(stem_dir, stem), stem_dir, e, g);
  from_branch->branch_ =
      get_connecting_node(ss.neighbour(branch_dir, branch), branch_dir, e, g);
}

void connect_nodes(track_element const& te, element::ptr const e, graph& g) {
  using enum track_element::nodes;
  using enum track_element::direction;

  auto node = get_node(e, one, g);
  node->next_ =
      get_connecting_node(te.neighbour(te.dir_, oneway), te.dir_, e, g);
}

void connect_nodes(cross const& c, element::ptr const e, graph& g) {
  using enum cross::direction;
  using enum cross::nodes;

  // determine if the cross starts its section or not
  auto const sl_pos = g.sections_.get_section_position(c.id_, start_left);
  auto const el_pos = g.sections_.get_section_position(c.id_, end_left);
  auto const sr_pos = g.sections_.get_section_position(c.id_, start_right);
  auto const er_pos = g.sections_.get_section_position(c.id_, end_right);

  // get mutable node refs
  auto sl_to_el_node = get_node(e, sl_to_el, g);
  auto el_to_sl_node = get_node(e, el_to_sl, g);
  auto sr_to_er_node = get_node(e, sr_to_er, g);
  auto er_to_sr_node = get_node(e, er_to_sr, g);

  // we have to create four edges
  //  auto from_sl = is_start(sl_pos) ? left_no : left_node;
  //  auto from_el = is_start(sl_pos) ? left_node : right_node;
  //  auto from_sr = is_start(sr_pos) ? bot_node : top_node;
  //  auto from_er = is_start(sr_pos) ? top_node : bot_node;

  // when the cross starts the sections or not we connect accordingly
  auto const sl_dir = section_pos_to_direction(sl_pos);
  auto const el_dir = section_pos_to_direction(el_pos);
  auto const sr_dir = section_pos_to_direction(sr_pos);
  auto const er_dir = section_pos_to_direction(er_pos);

  sl_to_el_node->next_ =
      get_connecting_node(c.neighbour(el_dir, end_left), el_dir, e, g);
  el_to_sl_node->next_ =
      get_connecting_node(c.neighbour(sl_dir, start_left), sl_dir, e, g);
  sr_to_er_node->next_ =
      get_connecting_node(c.neighbour(er_dir, end_right), er_dir, e, g);
  er_to_sr_node->next_ =
      get_connecting_node(c.neighbour(sr_dir, start_right), sr_dir, e, g);

  // if this is a cross switch we have to insert two more edges for every arc
  if (c.start_left_end_right_arc_) {
    //    auto f1 = is_start(sl_pos) ? lf_node : lr_node;
    auto f1 = sl_to_el_node;
    f1->branch_ =
        get_connecting_node(c.neighbour(er_dir, end_right), er_dir, e, g);

    //    auto f2 = is_start(er_pos) ? rf_node : rr_node;
    auto f2 = er_to_sr_node;
    f2->branch_ =
        get_connecting_node(c.neighbour(sl_dir, start_left), sl_dir, e, g);
  }

  if (c.start_right_end_left_arc_) {
    //    auto f1 = is_start(sr_pos) ? rf_node : rr_node;
    auto f1 = sr_to_er_node;
    f1->branch_ =
        get_connecting_node(c.neighbour(el_dir, end_left), el_dir, e, g);

    //    auto f2 = is_start(el_pos) ? lf_node : lr_node;
    auto f2 = el_to_sl_node;
    f2->branch_ =
        get_connecting_node(c.neighbour(sr_dir, start_right), sr_dir, e, g);
  }
}

void connect_nodes(element::ptr const e, graph& g) {
  return e->apply([&](auto&& x) { return connect_nodes(x, e, g); });
}

void connect_nodes(graph& g) {
  expects([&] {
    // no null neighbours allowed
    for (auto const& e : g.elements_) {
      expect(all_of(e->neighbours(), [](auto&& n) { return n != nullptr; }));
    }

    expect(g.sections_.ok(),
           "sections have to be completed before connecting nodes");
  });

  for (auto const& element : g.elements_) {
    connect_nodes(element, g);
  }

  for (auto& node : g.nodes_) {
    if (node->next_ != nullptr) {
      auto next_node = g.node_store_[as_val(node->next_->id_)].get();
      next_node->reverse_edges_.emplace_back(node);
    }

    if (node->branch_ != nullptr) {
      auto branch_node = g.node_store_[as_val(node->branch_->id_)].get();
      branch_node->reverse_edges_.emplace_back(node);
    }
  }

  ensures([&] {
    for (auto const& element : g.elements_) {
      ensure(all_of(element->nodes(),
                    [](auto&& n) { return n->next_ != nullptr; }) ||
             element->is_end_element());
    }
  });
}

void connect_border(border const& from_border, border const& to_border,
                    graph& g) {
  using enum mileage_dir;
  using enum simple_element::direction;

  auto from = g.element_store_[as_val(from_border.element_->get_id())].get();
  auto const to = to_border.element_;

  utls::expect(from != nullptr && from->is(type::BORDER));
  utls::expect(to != nullptr && to->is(type::BORDER));
  utls::expect(from_border.pos_ != section::position::middle);
  utls::expect(to_border.pos_ != section::position::middle);

  auto& from_as_se = from->as<simple_element>();

  if (from_border.pos_ == section::position::start) {
    detail::set_neighbour(from_as_se, rising, first, to);
    detail::set_neighbour(from_as_se, falling, first, to);
  } else {
    detail::set_neighbour(from_as_se, rising, second, to);
    detail::set_neighbour(from_as_se, falling, second, to);
  }
}

}  // namespace soro::infra