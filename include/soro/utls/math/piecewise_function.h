#pragma once

#include <cinttypes>

#include "utl/pairwise.h"
#include "utl/verify.h"

#include "soro/utls/math/polynomial.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/all_of.h"
#include "soro/utls/std_wrapper/find_if.h"
#include "soro/utls/template/is_vector.h"
#include "soro/utls/template/type_at_position.h"

namespace soro::utls {

template <typename Polynomial, typename InputType>
struct piece {
  using input_t = InputType;
  using result_t = typename Polynomial::template result_t<InputType>;

  auto operator<=>(piece const&) const = default;

  auto operator()(InputType const x) const { return piece_(x); }

  Polynomial piece_;
  InputType from_;
  InputType to_;
};

template <typename Polynomial, typename InputType>
auto make_piece(Polynomial poly_piece, InputType from, InputType to) {
  return piece<Polynomial, InputType>{
      .piece_ = poly_piece, .from_ = from, .to_ = to};
}

template <typename Pieces>
inline bool is_continuous(Pieces const& pieces) {
  return utls::all_of(utl::pairwise(pieces), [](auto&& pair) {
    if constexpr (std::is_floating_point_v<decltype(std::get<0>(pair).to_)> &&
                  std::is_floating_point_v<decltype(std::get<1>(pair).from_)>) {
      return soro::equal(std::get<0>(pair).to_, std::get<1>(pair).from_);
    } else {
      return std::get<0>(pair).to_ == std::get<1>(pair).from_;
    }
  });
}

template <typename PieceType>
struct piecewise_function {
  auto operator<=>(piecewise_function const&) const = default;

  template <typename InputType>
  auto operator()(InputType const x) const {
    auto const it =
        utls::find_if(pieces_, [&](auto&& p) { return x <= p.to_; });

    utl::verify(it != std::cend(pieces_),
                "could not find fitting piece from the piecewise function "
                "defined from {} to {} and x = {}",
                pieces_.front().from_, pieces_.back().to_, x);

    return (*it)(x);
  }

  soro::vector<PieceType> pieces_;
};

template <typename PieceType>
auto make_piecewise(soro::vector<PieceType>&& pieces) {
  utls::sassert(is_continuous(pieces),
                "Trying to make non continuous piecewise function!");

  return piecewise_function<PieceType>{.pieces_ = std::move(pieces)};
}

template <typename Piece, typename... Pieces,
          std::enable_if_t<!is_vector_v<decay_t<Piece>>, bool> = true>
auto make_piecewise(Piece&& piece, Pieces&&... pieces) {
  return make_piecewise(soro::vector<Piece>{std::forward<Piece>(piece),
                                            std::forward<Pieces>(pieces)...});
}

}  // namespace soro::utls
