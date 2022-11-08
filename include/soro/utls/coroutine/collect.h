#pragma once

namespace soro::utls {

template <typename Container, typename Generator>
Container emplace_back(Generator&& gen) {
  Container c;

  for (auto&& e : gen) {
    c.emplace_back(e);
  }

  return c;
}

}  // namespace soro::utls
