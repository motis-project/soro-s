#include "soro/infrastructure/interlocking/interlocking_route.h"

#include "utl/pipes.h"

#include "soro/utls/algo/overlap.h"

#include "soro/infrastructure/path/length.h"

namespace soro::infra {

type_set interlocking_route::valid_ends() {
  return type_set{{type::MAIN_SIGNAL, type::HALT, type::BORDER, type::BUMPER,
                   type::TRACK_END}};
}

// TODO(julian): this could use a vec -> iterable fix
std::vector<element_ptr> interlocking_route::elements(
    infrastructure const& infra) const {
  std::vector<element_ptr> elements;
  for (auto const sr_id : station_routes_) {
    auto const sr = infra->station_routes_[sr_id];
    utls::append(elements, soro::to_vec(sr->nodes(),
                                        [](auto&& n) { return n->element_; }));
  }
  return elements;
}

node::idx interlocking_route::size(infrastructure const& infra) const {
  node::idx result = 0;

  if (this->station_routes_.size() == 1) {
    return this->end_offset_ - this->start_offset_;
  }

  for (auto const sr : this->station_routes_) {
    result += infra->station_routes_[sr]->size();
  }

  result = result - start_offset_ -
           (infra->station_routes_[last_sr_id()]->size() - end_offset_);

  return result;
}

bool interlocking_route::contains(station_route::id const needle_sr,
                                  node::idx const needle_idx) const {

  if (station_routes_.size() == 1) {
    return station_routes_.front() == needle_sr &&
           start_offset_ <= needle_idx && needle_idx < end_offset_;
  }

  if (station_routes_.front() == needle_sr && start_offset_ <= needle_idx) {
    return true;
  }

  if (station_routes_.back() == needle_sr && needle_idx < end_offset_) {
    return true;
  }

  return std::any_of(std::begin(station_routes_) + 1,
                     std::end(station_routes_) - 1,
                     [&](auto&& sr_id) { return sr_id == needle_sr; });
}

station_route::id interlocking_route::first_sr_id() const {
  return this->station_routes_.front();
}

station_route::id interlocking_route::sr_id(sr_offset const sr_offset) const {
  return this->station_routes_[sr_offset];
}

station_route::id interlocking_route::last_sr_id() const {
  return this->station_routes_.back();
}

station_route::ptr interlocking_route::first_sr(
    infrastructure const& infra) const {
  return infra->station_routes_[this->first_sr_id()];
}

station_route::ptr interlocking_route::sr(sr_offset const sr_offset,
                                          infrastructure const& infra) const {
  return infra->station_routes_[this->sr_id(sr_offset)];
}

station_route::ptr interlocking_route::last_sr(
    infrastructure const& infra) const {
  return infra->station_routes_[this->last_sr_id()];
}

node::ptr interlocking_route::first_node(infrastructure const& infra) const {
  return first_sr(infra)->nodes(start_offset_);
}

node::ptr interlocking_route::last_node(infrastructure const& infra) const {
  return last_sr(infra)->nodes(end_offset_ - 1);
}

bool interlocking_route::starts_on_ms(infrastructure const& infra) const {
  return first_node(infra)->is(type::MAIN_SIGNAL);
}

bool interlocking_route::ends_on_ms(infrastructure const& infra) const {
  return last_node(infra)->is(type::MAIN_SIGNAL);
}

utls::it_range<utls::id_it_ptr<station_route>>
interlocking_route::station_routes(infrastructure const& infra) const {
  return utls::make_range(
      utls::id_iterator(std::begin(station_routes_), &infra->station_routes_),
      utls::id_iterator(std::end(station_routes_), &infra->station_routes_));
}

bool interlocking_route::follows(interlocking_route const& other,
                                 infrastructure const& infra) const {
  return this->first_node(infra) == other.last_node(infra);
}

bool interlocking_route::operator==(interlocking_route const& o) const {
  return this->id_ == o.id_;
}

namespace detail {

utls::recursive_generator<route_node> from_to(
    interlocking_route::ptr const ir,
    decltype(ir->station_routes_)::const_iterator from_it, node::idx const from,
    decltype(ir->station_routes_)::const_iterator to_it, node::idx const to,
    infrastructure const& infra) {

  utls::sassert(ir->station_routes_.size() > 1,
                "Called from_to_impl with a single station route in IR {}, "
                "call from_to_single_imple instead",
                ir->id_);
  utls::sassert(from_it != std::end(ir->station_routes_),
                "Station route {} is not part of interlocking route {}, but "
                "got it for iteration.",
                *from_it, ir->id_);
  utls::sassert(to_it != std::end(ir->station_routes_),
                "Station route {} is not part of interlocking route {}, but "
                "got it for iteration.",
                *to_it, ir->id_);
  utls::sassert(std::distance(from_it, to_it) >= 0,
                "To station route is located before from station route in "
                "interlocking route iterator.");
  utls::sassert(*from_it != *to_it,
                "Don't call this with from == to, "
                "since it will yield the wrong nodes.");

  co_yield infra->station_routes_[*from_it]->from(from);
  ++from_it;

  for (; from_it != to_it; ++from_it) {
    co_yield infra->station_routes_[*from_it]->iterate();
  }

  co_yield infra->station_routes_[*to_it]->to(to);
}

}  // namespace detail

utls::recursive_generator<route_node> interlocking_route::from_to(
    station_route::id const from_sr, node::idx const from,
    station_route::id const to_sr, node::idx const to,
    infrastructure const& infra) const {

  if (this->station_routes_.size() == 1) {
    utls::sassert(
        from_sr == to_sr,
        "Only one station route in interlocking route {}, but while iterating "
        "got from {} and to {}.",
        this->id_, from_sr, to_sr);

    return this->first_sr(infra)->from_to(from, to);
  } else {
    auto from_it = utls::find(this->station_routes_, from_sr);
    auto to_it = utls::find(this->station_routes_, to_sr);

    utls::sassert(from_it != std::end(this->station_routes_),
                  "Station route {} is not part of interlocking route {}, but "
                  "got it for iteration.",
                  from_sr, this->id_);
    utls::sassert(to_it != std::end(this->station_routes_),
                  "Station route {} is not part of interlocking route {}, but "
                  "got it for iteration.",
                  to_sr, this->id_);
    utls::sassert(std::distance(from_it, to_it) >= 0,
                  "To station route is located before from station route in "
                  "interlocking route iterator.");

    return *from_it == *to_it
               ? infra->station_routes_[*from_it]->from_to(from, to)
               : detail::from_to(this, from_it, from, to_it, to, infra);
  }
}

utls::recursive_generator<route_node> interlocking_route::to(
    station_route::id const sr_id, node::idx const to,
    infrastructure const& infra) const {
  return this->from_to(this->station_routes_.front(), start_offset_, sr_id, to,
                       infra);
}

utls::recursive_generator<route_node> interlocking_route::from(
    station_route::id const sr_id, node::idx const from,
    infrastructure const& infra) const {
  return this->from_to(sr_id, from, station_routes_.back(), end_offset_, infra);
}

utls::recursive_generator<route_node> interlocking_route::iterate(
    infrastructure const& infra) const {
  if (station_routes_.size() == 1) {
    return this->first_sr(infra)->from_to(start_offset_, end_offset_);
  } else {
    return detail::from_to(this, std::cbegin(station_routes_), start_offset_,
                           std::cend(station_routes_) - 1, end_offset_, infra);
  }
}

utls::generator<sub_path> interlocking_route::iterate_station_routes(
    infrastructure_t const& infra) const {
  if (station_routes_.size() == 1) {
    auto const sr = infra.station_routes_[station_routes_.front()];
    co_yield sub_path{
        .station_route_ = sr, .from_ = start_offset_, .to_ = end_offset_};
  } else {
    auto const first_sr = infra.station_routes_[station_routes_.front()];
    co_yield sub_path{.station_route_ = first_sr,
                      .from_ = start_offset_,
                      .to_ = first_sr->size()};

    for (soro::size_t i = 1; i < station_routes_.size() - 1; ++i) {
      auto const sr = infra.station_routes_[station_routes_[i]];
      co_yield sub_path{.station_route_ = sr, .from_ = 0, .to_ = sr->size()};
    }

    auto const last_sr = infra.station_routes_[station_routes_.back()];
    co_yield sub_path{
        .station_route_ = last_sr, .from_ = 0, .to_ = end_offset_};
  }
}

}  // namespace soro::infra