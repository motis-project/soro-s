#pragma once

#include "soro/utls/concepts/is_any_of.h"

namespace soro::utls {

template <typename Result, typename Iterable>
concept yields =
    requires(Iterable i) {
      { *i.begin() } -> is_any_of<Result, Result&, Result const, Result const&>;

      {
        *std::begin(i)
        } -> is_any_of<Result, Result&, Result const, Result const&>;

      { *std::as_const(i).begin() } -> is_any_of<Result const, Result const&>;
      { *std::cbegin(i) } -> is_any_of<Result const, Result const&>;
    };

}  // namespace soro::utls
