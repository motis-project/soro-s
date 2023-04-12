#include "soro/server/modules/search/search_module.h"

#include "range/v3/view/transform.hpp"

#include "utl/concat.h"
#include "utl/enumerate.h"
#include "utl/timer.h"

#include "soro/utls/coroutine/coro_map.h"

namespace soro::server {

std::string left_pad_to_3_digits(std::integral auto const& i) {
  auto const id = std::to_string(i);
  return std::string(3 - std::min(3UL, id.length()), '0') + id;
}

utls::bounding_box get_bounding_box(
    infra::station::ptr const s, soro::vector<utls::gps> const& element_pos) {
  return utls::get_bounding_box(
      s->elements_ |
      ranges::views::transform([&](auto&& e) { return element_pos[e->id()]; }));
}

utls::bounding_box get_bounding_box(
    infra::element::ptr const e, soro::vector<utls::gps> const& element_pos) {
  return utls::get_bounding_box(std::array<utls::gps, 1>{element_pos[e->id()]});
}

utls::bounding_box get_bounding_box(
    infra::station_route::ptr const s,
    soro::vector<utls::gps> const& element_pos) {
  return utls::get_bounding_box(s->nodes() |
                                ranges::views::transform([&](auto&& n) {
                                  return element_pos[n->element_->id()];
                                }));
}

utls::bounding_box get_bounding_box(infra::interlocking_route const& r,
                                    infra::infrastructure const& infra) {
  return utls::get_bounding_box(
      utls::coro_map(r.iterate(infra), [&](auto&& rn) {
        return infra->element_positions_[rn.node_->element_->id()];
      }));
}

search_module::context get_search_context(infra::infrastructure const& infra) {
  using namespace infra;

  search_module::context ctx;

  // add all station ds100s
  utl::concat(ctx.results_, utl::to_vec(infra->stations_, [&](auto&& s) {
                return search_module::result{
                    search_module::result::type::STATION, s->ds100_,
                    get_bounding_box(s, infra->element_positions_), s->id_};
              }));

  // add all station full names
  for (auto const [id, full] : utl::enumerate(infra->full_station_names_)) {
    auto const s = infra->stations_[id];

    if (s->ds100_ == full) {
      continue;
    }

    ctx.results_.push_back(search_module::result{
        search_module::result::type::STATION, full,
        get_bounding_box(s, infra->element_positions_), id});
  }

  // add all elements
  for (auto const& e : infra->graph_.elements_) {
    auto const bb = get_bounding_box(e, infra->element_positions_);
    auto const type = search_module::result::type::ELEMENT;
    auto const id = e->id();
    auto const type_str = get_type_str(e->type());

    ctx.results_.push_back({type, left_pad_to_3_digits(id), bb, id, type_str});

    if (e->is(type::MAIN_SIGNAL)) {
      auto const data = infra->graph_.element_data_[id].as<main_signal>();

      ctx.results_.push_back({type, data.name_, bb, id, type_str});
    }

    if (e->is(type::SIMPLE_SWITCH)) {
      auto const data = infra->graph_.element_data_[id].as<switch_data>();

      ctx.results_.push_back({type, data.name_, bb, id, type_str});
    }

    if (e->is(type::HALT)) {
      auto const data = infra->graph_.element_data_[id].as<halt>();

      ctx.results_.push_back({type, data.name_, bb, id, type_str});
      ctx.results_.push_back({type, data.identifier_operational_, bb, id});
      ctx.results_.push_back({type, data.identifier_extern_, bb, id, type_str});
    }
  }

  // add all station routes
  utl::concat(ctx.results_, utl::to_vec(infra->station_routes_, [&](auto&& r) {
                return search_module::result{
                    search_module::result::type::STATION_ROUTE, r->name_,
                    get_bounding_box(r, infra->element_positions_), r->id_};
              }));

  // add all interlocking routes
  utl::concat(
      ctx.results_, utl::to_vec(infra->interlocking_.routes_, [&](auto&& r) {
        return search_module::result{
            search_module::result::type::INTERLOCKING_ROUTE,
            left_pad_to_3_digits(r.id_), get_bounding_box(r, infra), r.id_};
      }));

  // index all search results in one single guesser
  ctx.guesser_ =
      std::make_unique<guess::guesser>(utl::to_vec(ctx.results_, [](auto&& r) {
        return std::pair{r.name_, 1.0F};
      }));

  return ctx;
}

search_module get_search_module(infrastructure_module const& infra_m) {
  utl::scoped_timer const timer("creating search module");

  search_module mod;

  for (auto const& infra : infra_m.all()) {
    mod.contexts_.emplace(infra->source_, get_search_context(infra));
  }

  return mod;
}

}  // namespace soro::server