#pragma once

#include "soro/utls/concepts/iterable_helpers.h"
#include "soro/utls/concepts/iterable_yields.h"
#include "soro/utls/coroutine/coro_map.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/path/is_path.h"
#include "soro/si/units.h"

namespace soro::infra {

template <typename Iterable>
  requires utls::yields<element_ptr, Iterable> &&
           utls::is_input_iterable<Iterable>
si::length get_path_length_from_elements(Iterable&& element_iter) {
  si::length distance = si::ZERO<si::length>;

  element_ptr last_element = *std::begin(element_iter);
  for (auto const& current_element : element_iter) {
    auto const kmp = last_element->get_km(current_element);
    auto const next_kmp = current_element->get_km(last_element);

    distance += abs(kmp - next_kmp);

    last_element = current_element;
  }

  return distance;
}

template <typename Iterable>
  requires utls::yields<node_ptr, Iterable> && utls::is_input_iterable<Iterable>
si::length get_path_length_from_elements(Iterable&& node_iter) {
  return get_path_length_from_elements(utls::coro_map(
      node_iter, [](auto&& node_ptr) { return node_ptr->element_; }));
}

template <typename Iterable>
  requires utls::yields<element_ptr, Iterable> &&
           utls::is_input_iterable<Iterable>
si::length get_path_length_from_sections(Iterable&& element_iter) {
  auto iter = std::begin(element_iter);

  si::length distance = si::ZERO<si::length>;

  element_ptr last_section = *iter;
  element_ptr next_to_last_section = *(++iter);

  element_ptr next_section = next_to_last_section;
  element_ptr prev_to_next_section = last_section;

  while (iter != std::end(element_iter)) {
    while (iter != std::end(element_iter) && (*iter)->is_track_element()) {
      ++iter;

      prev_to_next_section = next_section;
      next_section = *iter;
    }

    distance += abs(last_section->get_km(next_to_last_section) -
                    next_section->get_km(prev_to_next_section));

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
  requires utls::yields<node_ptr, Iterable> && utls::is_input_iterable<Iterable>
si::length get_path_length_from_sections(Iterable&& node_iter) {
  return get_path_length_from_sections(utls::coro_map(
      node_iter, [](auto&& node_ptr) { return node_ptr->element_; }));
}

}  // namespace soro::infra