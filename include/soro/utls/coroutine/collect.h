#pragma once

namespace soro::utls {

template <typename Container, typename Generator>
Container collect(Generator&& gen) {
  Container c;

  for (auto&& e : gen) {
    c.emplace_back(e);
  }

  return c;
}

template <typename Container, typename Generator, typename Fn>
Container collect(Generator&& gen, Fn&& fn) {
  Container c;

  for (auto&& e : gen) {
    c.emplace_back(fn(e));
  }

  return c;
}

}  // namespace soro::utls
