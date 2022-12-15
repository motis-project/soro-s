#pragma once

#include <cinttypes>

#include "utl/pairwise.h"
#include "utl/verify.h"

#include "soro/base/fp_precision.h"

#include "soro/utls/math/polynomial.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/std_wrapper.h"
#include "soro/utls/template/is_vector.h"
#include "soro/utls/template/type_at_position.h"

namespace soro::utls {

template <typename Polynomial, typename InputType>
struct piece {
  using input_t = InputType;
  using result_t = typename Polynomial::template result_t<InputType>;

  auto operator()(InputType const x) const { return piece_(x); }

  Polynomial piece_;
  InputType from_{};
  InputType to_{};
};

template <typename Polynomial, typename InputType>
auto make_piece(Polynomial poly_piece, InputType from, InputType to) {
  return piece<Polynomial, InputType>{
      .piece_ = poly_piece, .from_ = from, .to_ = to};
}

template <typename Pieces>
inline bool is_continuous(Pieces const& pieces) {
  using soro::equal;
  using std::abs;

  return utls::all_of(utl::pairwise(pieces), [](auto&& pair) {
    return equal(abs(std::get<0>(pair).to_ - std::get<1>(pair).from_), 0.0);
  });
}

template <typename PieceType>
struct piecewise_function {

  template <typename InputType>
  auto operator()(InputType const x) const {
    auto it = std::cend(pieces_);
    if constexpr (std::is_floating_point_v<InputType>) {
      it = utls::find_if(pieces_, [&](auto const& p) { return x < p.to_; });
    } else {
      it = utls::find_if(pieces_,
                         [&](auto const& p) { return x.val_ < p.to_.val_; });
    }

    utl::verify(it != std::cend(pieces_),
                "Could not find fitting piece from the piecewise function "
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
