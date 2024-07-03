#include "test/infrastructure/graph_test.h"

#include <iterator>
#include <utility>

#include "doctest/doctest.h"

#include "utl/verify.h"

#include "soro/base/soro_types.h"

#include "soro/utls/narrow.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/accumulate.h"
#include "soro/utls/std_wrapper/all_of.h"
#include "soro/utls/std_wrapper/find_if.h"

#include "soro/infrastructure/graph/construction/starts_section.h"
#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/graph.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/kilometrage.h"

namespace soro::infra::test {

soro::size_t get_expected_outgoing_edges(element::ptr const e) {
  if (e->is_track_element() || e->is_end_element()) {
    return 1;
  }

  if (e->is_simple_element()) {
    return 2;
  }

  if (e->is(type::SIMPLE_SWITCH)) {
    return 3;
  }

  if (e->is(type::CROSS)) {
    auto const& c = e->as<cross>();
    return 4 + utls::narrow<soro::size_t>(c.start_left_end_right_arc_) * 2 +
           utls::narrow<soro::size_t>(c.start_right_end_left_arc_) * 2;
  }

  utls::unreachable("element is invalid?");

  return 0;
}

void check_switch(simple_switch const& ss, graph const& g) {
  using enum mileage_dir;
  using enum simple_switch::direction;

  auto const get_node = [](element const& e, element::id const id) {
    for (auto const& n : e.nodes()) {
      if (n->next_ != nullptr && n->next_->element_->get_id() == id) {
        return std::pair(n, false);
      }

      if (n->branch_ != nullptr && n->branch_->element_->get_id() == id) {
        return std::pair(n, true);
      }
    }

    throw utl::fail(
        "Could not find the correct node from the neighbour to the switch!");
  };

  auto const start_pos = g.sections_.get_section_position(ss.id_, start);
  auto const stem_pos = g.sections_.get_section_position(ss.id_, stem);
  auto const branch_pos = g.sections_.get_section_position(ss.id_, branch);

  auto const start_dir = section_pos_to_direction(start_pos);
  auto const stem_dir = section_pos_to_direction(stem_pos);
  auto const branch_dir = section_pos_to_direction(branch_pos);

  {  // check start -> stem and start -> branch
    auto const& [node, take_branch] =
        get_node(*ss.neighbour(opposite(start_dir), start), ss.id_);

    auto const stem_id = (take_branch ? node->branch_ : node->next_)  // NOLINT
                             ->next_->element_->get_id();

    CHECK_EQ(stem_id, ss.neighbour(stem_dir, stem)->get_id());

    auto const branch_id = (take_branch ? node->branch_ : node->next_)
                               ->branch_->element_->get_id();
    CHECK_EQ(branch_id, ss.neighbour(branch_dir, branch)->get_id());
  }

  {  // check stem -> start
    auto const& [node, take_branch] =
        get_node(*ss.neighbour(opposite(stem_dir), stem), ss.id_);

    auto const start_id =
        (take_branch ? node->branch_ : node->next_)->next_->element_->get_id();

    CHECK_EQ(start_id, ss.neighbour(start_dir, start)->get_id());
  }

  {  // check branch -> start
    auto const& [node, take_branch] =
        get_node(*ss.neighbour(opposite(branch_dir), branch), ss.id_);

    auto const start_id =
        (take_branch ? node->branch_ : node->next_)->next_->element_->get_id();

    CHECK_EQ(start_id, ss.neighbour(start_dir, start)->get_id());
  }
}

void check_cross(cross const& c, graph const& g) {
  using enum mileage_dir;
  using enum cross::direction;

  auto const get_cross_node = [](element const& e, element::id const id) {
    auto it = utls::find_if(e.nodes(), [&](auto&& n) {
      return n->next_ != nullptr && n->next_->element_->get_id() == id;
    });
    if (it != std::end(e.nodes())) return (*it)->next_;

    it = utls::find_if(e.nodes(), [&](auto&& n) {
      return n->branch_ != nullptr && n->branch_->element_->get_id() == id;
    });
    if (it != std::end(e.nodes())) return (*it)->branch_;

    utls::unreachable(
        "could not find the correct node from the neighbour to the switch!");

    return node::ptr{};
  };

  auto const sl_pos = g.sections_.get_section_position(c.id_, start_left);
  auto const el_pos = g.sections_.get_section_position(c.id_, end_left);
  auto const sr_pos = g.sections_.get_section_position(c.id_, start_right);
  auto const er_pos = g.sections_.get_section_position(c.id_, end_right);

  auto const sl_dir = section_pos_to_direction(sl_pos);
  auto const el_dir = section_pos_to_direction(el_pos);
  auto const sr_dir = section_pos_to_direction(sr_pos);
  auto const er_dir = section_pos_to_direction(er_pos);

  {  // check start_left -> end_left and start_left -> end_right
    auto const cross_node =
        get_cross_node(*c.neighbour(opposite(sl_dir), start_left), c.id_);

    auto const end_left_id = cross_node->next_->element_->get_id();

    CHECK_EQ(end_left_id, c.neighbour(el_dir, end_left)->get_id());

    if (c.start_left_end_right_arc_) {
      auto const end_right_id = cross_node->branch_->element_->get_id();

      CHECK_EQ(end_right_id, c.neighbour(er_dir, end_right)->get_id());
    }
  }

  {  // check end_left -> start_left and end_left -> start_right
    auto const cross_node =
        get_cross_node(*c.neighbour(opposite(el_dir), end_left), c.id_);

    auto const start_left_id = cross_node->next_->element_->get_id();

    CHECK_EQ(start_left_id, c.neighbour(sl_dir, start_left)->get_id());

    if (c.start_right_end_left_arc_) {
      auto const start_right_id = cross_node->branch_->element_->get_id();

      CHECK_EQ(start_right_id, c.neighbour(sr_dir, start_right)->get_id());
    }
  }

  {  // check start_right -> end_right and start_right -> end_left
    auto const cross_node =
        get_cross_node(*c.neighbour(opposite(sr_dir), start_right), c.id_);

    auto const end_right_id = cross_node->next_->element_->get_id();
    CHECK_EQ(end_right_id, c.neighbour(er_dir, end_right)->get_id());

    if (c.start_right_end_left_arc_) {
      auto const end_left_id = cross_node->branch_->element_->get_id();
      CHECK_EQ(end_left_id, c.neighbour(el_dir, end_left)->get_id());
    }
  }

  {  // check end_right -> start_right and end_right -> start_left
    auto const cross_node =
        get_cross_node(*c.neighbour(opposite(er_dir), end_right), c.id_);

    auto const start_right_id = cross_node->next_->element_->get_id();
    CHECK_EQ(start_right_id, c.neighbour(sr_dir, start_right)->get_id());

    if (c.start_left_end_right_arc_) {
      auto const start_left_id = cross_node->branch_->element_->get_id();
      CHECK_EQ(start_left_id, c.neighbour(sl_dir, start_left)->get_id());
    }
  }
}

void check_outgoing(element::ptr const e) {
  auto const outgoing =
      utls::accumulate(e->nodes(), soro::size_t{0}, [](auto&& acc, auto&& n) {
        return acc + utls::narrow<soro::size_t>(n->next_ != nullptr) +
               utls::narrow<soro::size_t>(n->branch_ != nullptr);
      });
  auto const expected = get_expected_outgoing_edges(e);

  CHECK_EQ(outgoing, expected);
}

soro::vector_map<element::id, soro::size_t> get_incoming_edges(graph const& g) {
  soro::vector_map<element::id, soro::size_t> result(g.elements_.size(), 0);

  for (auto const& node : g.nodes_) {
    if (node->next_ != nullptr) {
      ++(result[node->next_->element_->get_id()]);
    }

    if (node->branch_ != nullptr) {
      ++(result[node->branch_->element_->get_id()]);
    }
  }

  return result;
}

void check_neighbours(element::ptr const e) {
  CHECK(utls::all_of(e->neighbours(), [](auto&& n) { return n != nullptr; }));
}

void check_no_trivial_cycle(graph const& graph) {
  for (auto const& node : graph.nodes_) {
    // if there is a -> b, then b -> a should not exist
    auto const no_next_cycle =
        node->next_ == nullptr ||
        (node->next_->next_ != node && node->next_->branch_ != node);
    CHECK(no_next_cycle);

    auto const no_branch_cycle =
        node->branch_ == nullptr ||
        (node->branch_->next_ != node && node->branch_->branch_ != node);
    CHECK(no_branch_cycle);
  }
}

void check_graph(graph const& g) {
  using enum mileage_dir;
  using enum cross::direction;

  for (auto const& element : g.elements_) {
    check_neighbours(element);
    check_outgoing(element);

    if (element->is(type::SIMPLE_SWITCH)) {
      check_switch(element->as<simple_switch>(), g);
    }

    if (element->is(type::CROSS)) {
      check_cross(element->as<cross>(), g);
    }
  }

  check_no_trivial_cycle(g);
}

}  // namespace soro::infra::test
