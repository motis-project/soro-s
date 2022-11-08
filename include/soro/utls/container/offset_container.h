#pragma once

#include <cstdint>
#include <numeric>
#include <vector>

#include "soro/base/soro_types.h"

namespace soro::utls {

template <typename Container, typename SizeType = std::size_t>
struct offset_container {
  using size_type = SizeType;

  offset_container() = default;

  offset_container(size_type const offset, size_type const size)
      : offset_{offset}, vec_{size} {}

  auto& operator[](size_type const idx) noexcept { return vec_[idx - offset_]; }

  auto const& operator[](size_type const idx) const noexcept {
    return vec_[idx - offset_];
  }

  auto resize(size_type const new_size) { vec_.resize(new_size); }

private:
  size_type offset_{std::numeric_limits<size_type>::max()};
  Container vec_{};
};

}  // namespace soro::utls
