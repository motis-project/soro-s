#include "soro/server/modules/search/search_module.h"

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <utility>

#include "range/v3/view/transform.hpp"

#include "guess/guesser.h"

#include "utl/concat.h"
#include "utl/enumerate.h"
#include "utl/timer.h"
#include "utl/to_vec.h"

#include "soro/base/soro_types.h"

#include "soro/utls/coordinates/gps.h"
#include "soro/utls/coroutine/coro_map.h"

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/element_data.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"
#include "soro/infrastructure/station/station.h"

#include "soro/server/modules/infrastructure/infrastructure_module.h"
#include "soro/server/modules/infrastructure/positions.h"

namespace soro::server {

using namespace soro::infra;

std::string left_pad_to_3_digits(std::integral auto const i) {
  auto const id = std::to_string(i);
  return std::string(
             3UL - std::min(3UL, static_cast<decltype(1UL)>(id.length())),
             '0') +
         id;
}

utls::bounding_box get_bounding_box(
    station::ptr const s,
    soro::vector_map<element::id, utls::gps> const& element_pos) {
  return utls::get_bounding_box(s->elements_ |
                                ranges::views::transform([&](auto&& e) {
                                  return element_pos[e->get_id()];
                                }));
}

utls::bounding_box get_bounding_box(
    element::ptr const e,
    soro::vector_map<element::id, utls::gps> const& element_pos) {
  return utls::get_bounding_box(
      std::array<utls::gps, 1>{element_pos[e->get_id()]});
}

utls::bounding_box get_bounding_box(
    station_route::ptr const s,
    soro::vector_map<element::id, utls::gps> const& element_pos) {
  return utls::get_bounding_box(s->nodes() |
                                ranges::views::transform([&](auto&& n) {
                                  return element_pos[n->element_->get_id()];
                                }));
}

utls::bounding_box get_bounding_box(interlocking_route const& r,
                                    infrastructure const& infra,
                                    positions const& positions) {
  return utls::get_bounding_box(
      utls::coro_map(r.iterate(infra), [&](auto&& rn) {
        return positions.elements_[rn.node_->element_->get_id()];
      }));
}

search_module::context get_search_context(infrastructure const& infra,
                                          positions const& positions) {
  search_module::context ctx;

  // add all station ds100s
  utl::concat(ctx.results_, utl::to_vec(infra->stations_, [&](auto&& s) {
                std::string const name(s->ds100_);
                return search_module::result{
                    search_module::result::type::STATION, name,
                    get_bounding_box(s, positions.elements_), as_val(s->id_)};
              }));

  // add all station full names
  for (auto const [id, full] :
       utl::enumerate<station::id>(infra->full_station_names_)) {
    auto const s = infra->stations_[id];

    if (s->ds100_ == full) {
      continue;
    }

    std::string const name(full);
    ctx.results_.push_back(search_module::result{
        search_module::result::type::STATION, name,
        get_bounding_box(s, positions.elements_), as_val(id)});
  }

  // add all elements
  for (auto const& e : infra->graph_.elements_) {
    auto const bb = get_bounding_box(e, positions.elements_);
    auto const type = search_module::result::type::ELEMENT;
    auto const id = as_val(e->get_id());
    std::string const type_str(get_type_str(e->type()).data());  // NOLINT

    ctx.results_.push_back({type, left_pad_to_3_digits(id), bb, id, type_str});

    if (e->is(type::MAIN_SIGNAL)) {
      auto const data = infra->graph_.get_element_data<main_signal>(e);
      std::string const name(data.name_);

      ctx.results_.push_back({type, name, bb, id, type_str});
    }

    if (e->is(type::SIMPLE_SWITCH)) {
      auto const data = infra->graph_.get_element_data<switch_data>(e);
      std::string const name(data.name_);

      ctx.results_.push_back({type, name, bb, id, type_str});
    }

    if (e->is(type::HALT)) {
      auto const data = infra->graph_.get_element_data<halt>(e);

      if (!data.name_.empty()) {
        std::string const name(data.name_);
        ctx.results_.push_back({type, name, bb, id, type_str});
      }

      if (!data.identifier_extern_.empty()) {
        std::string const name(data.identifier_operational_);
        ctx.results_.push_back({type, name, bb, id});
      }

      if (!data.identifier_operational_.empty()) {
        std::string const name(data.identifier_extern_);
        ctx.results_.push_back({type, name, bb, id, type_str});
      }
    }
  }

  // add all station routes
  // index via name
  utl::concat(ctx.results_, utl::to_vec(infra->station_routes_, [&](auto&& r) {
                std::string const name(r->name_);
                return search_module::result{
                    search_module::result::type::STATION_ROUTE, name,
                    get_bounding_box(r, positions.elements_), as_val(r->id_)};
              }));

  // index via id
  utl::concat(ctx.results_, utl::to_vec(infra->station_routes_, [&](auto&& r) {
                return search_module::result{
                    search_module::result::type::STATION_ROUTE,
                    left_pad_to_3_digits(as_val(r->id_)),
                    get_bounding_box(r, positions.elements_), as_val(r->id_)};
              }));

  // add all interlocking routes
  utl::concat(ctx.results_,
              utl::to_vec(infra->interlocking_.routes_, [&](auto&& r) {
                return search_module::result{
                    search_module::result::type::INTERLOCKING_ROUTE,
                    left_pad_to_3_digits(as_val(r.id_)),
                    get_bounding_box(r, infra, positions), as_val(r.id_)};
              }));

  // index all search results in one single guesser
  ctx.guesser_ = std::make_unique<guess::guesser>(utl::to_vec(
      ctx.results_, [](auto&& r) { return std::pair{r.name_, 1.0F}; }));

  return ctx;
}

search_module get_search_module(infrastructure_module const& infra_m) {
  utl::scoped_timer const timer("creating search module");

  search_module mod;

  for (auto const& context : infra_m.all()) {
    mod.contexts_.emplace(
        context.infra_->source_,
        get_search_context(context.infra_, context.positions_));
  }

  return mod;
}

}  // namespace soro::server