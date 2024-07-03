#pragma once

#include "soro/utls/concepts/iterable_helpers.h"
#include "soro/utls/concepts/yields.h"
#include "soro/utls/coroutine/coro_map.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/path/is_path.h"
#include "soro/si/units.h"

namespace soro::infra {

template <typename Iterable>
  requires utls::yields<element::ptr, Iterable> &&
           utls::is_input_iterable<Iterable>
si::length get_path_length_from_elements(Iterable&& element_iter) {
  auto distance = si::length::zero();

  auto it = std::begin(element_iter);

  element::ptr last_element = *it;
  ++it;

  for (; it != std::end(element_iter); ++it) {
    auto const kmp = last_element->km(*it);
    auto const next_kmp = (*it)->km(last_element);

    distance += (kmp - next_kmp).abs();

    last_element = *it;
  }

  return distance;
}

template <typename Iterable>
  requires utls::yields<node::ptr, Iterable> &&
           utls::is_input_iterable<Iterable>
si::length get_path_length_from_elements(Iterable&& node_iter) {
  return get_path_length_from_elements(utls::coro_map(
      node_iter, [](node::ptr node_ptr) { return node_ptr->element_; }));
}

template <typename Iterable>
  requires utls::yields<element::ptr, Iterable> &&
           utls::is_input_iterable<Iterable>
si::length get_path_length_from_sections(Iterable&& element_iter) {
  auto iter = std::begin(element_iter);

  auto distance = si::length::zero();

  element::ptr last_section = *iter;
  element::ptr next_to_last_section = *(++iter);

  element::ptr next_section = next_to_last_section;
  element::ptr prev_to_next_section = last_section;

  while (iter != std::end(element_iter)) {
    while (iter != std::end(element_iter) && (*iter)->is_track_element()) {
      ++iter;

      prev_to_next_section = next_section;
      next_section = *iter;
    }

    distance += (last_section->km(next_to_last_section) -
                 next_section->km(prev_to_next_section))
                    .abs();

    if (iter == std::end(element_iter)) {
      break;
    }

    ++iter;

    last_section = next_section;
    next_to_last_section = *iter;

    prev_to_next_section = next_section;
    next_section = *iter;
  }

  return distance;
}

template <typename Iterable>
  requires utls::yields<node::ptr, Iterable> &&
           utls::is_input_iterable<Iterable>
si::length get_path_length_from_sections(Iterable&& node_iter) {
  return get_path_length_from_sections(utls::coro_map(
      node_iter, [](auto&& node_ptr) { return node_ptr->element_; }));
}

}  // namespace soro::infra