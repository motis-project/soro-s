#pragma once

#include <cmath>

#include "soro/si/units.h"

namespace soro::runtime {

/**
 * Implementation of the Golden Section Search Algorithm to determine the
 * maximum of a function in a given range (xlow, xhigh).
 *
 * @see https://en.wikipedia.org/wiki/Golden-section_search
 *
 * @param xlow current/initial range start; speed value
 * @param xhigh current/initial range end; speed value
 * @param err allowed uncertainty range in which gss must find the maximum of
 * func.
 * @param func an arbitrary function to be maximized. speed -> time
 * @return the speed at which func seems to have its maximum.
 */
template <typename X, typename Y>
X golden_section_search_max(X xlow, X xhigh, X err, Y (*func)(X)) {
  // define d lambda expression: d = 0.5 (sqrt(5) - 1) * (xhigh - xlow)
  auto d = [](X low, X high) { return 0.5 * (sqrt(5.0) - 1.0) * (high - low); };

  // let x1 = xlow + d and x2 = xhigh - d
  X x1 = xlow + d(xlow, xhigh);
  X x2 = xhigh - d(xlow, xhigh);

  // while xhigh - xlow > err: update x1, x2, xhigh, xlow
  while (xhigh - xlow > err) {
    if (func(x1) > func(x2)) {
      xlow = x2;
      x2 = x1;
      x1 = xlow + d(xlow, xhigh);
    } else {
      xhigh = x1;
      x1 = x2;
      x2 = xhigh - d(xlow, xhigh);
    }
  }

  // determine x as 0.5 * (xhigh + xlow)
  return 0.5 * (xhigh + xlow);
}

/**
 * Implementation of the Golden Section Search Algorithm to determine the
 * minimum of a function in a given range (xlow, xhigh).
 *
 * @note see golden_section_search_max
 * @see https://en.wikipedia.org/wiki/Golden-section_search
 *
 * @return the speed at which func seems to have its minimum.
 */
template <typename X, typename Y>
X golden_section_search_min(X xlow, X xhigh, X err, Y (*func)(X)) {
  // define d lambda expression: d = 0.5 * (sqrt(5) - 1) * (xhigh - xlow)
  auto d = [](X low, X high) { return 0.5 * (sqrt(5.0) - 1.0) * (high - low); };

  // let x1 = xlow + d and x2 = xhigh - d
  X x1 = xlow + d(xlow, xhigh);
  X x2 = xhigh - d(xlow, xhigh);

  // while xhigh - xlow > err: update x1, x2, xhigh, xlow
  while (xhigh - xlow > err) {
    if (func(x1) <= func(x2)) {
      xlow = x2;
      x2 = x1;
      x1 = xlow + d(xlow, xhigh);
    } else {
      xhigh = x1;
      x1 = x2;
      x2 = xhigh - d(xlow, xhigh);
    }
  }

  // determine x as 0.5 * (xhigh + xlow)
  return 0.5 * (xhigh + xlow);
}

/**
 * Implementation of the Golden Section Search Algorithm to determine either the
 * maximum or the minimum of a function in a given range (xlow, xhigh)
 *
 * @see `golden_section_search_max`
 * @see https://en.wikipedia.org/wiki/Golden-section_search
 *
 * @param search_min chooses, whether the minimum or the maximum is searched,
 * default: true, therefore the minimum is searched.
 *
 * @return the speed at which the function has its minimum or maximum.
 */
template <typename X, typename Y>
X golden_section_search(X xlow, X xhigh, X err, Y (*func)(X),
                        bool search_min = true) {
  if (search_min) {
    return golden_section_search_min(xlow, xhigh, err, func);
  }
  return golden_section_search_max(xlow, xhigh, err, func);
};

/**
 * @brief Concrete implementation of the Golden Section Search Algorithm to
 * determine the minimum of a optimizer function defined in cruising_candidates.
 *
 * @see `golden_section_search_max/min`
 * @see https://en.wikipedia.org/wiki/Golden-section_search
 *
 * @remark: optimizer must implement a get_at(X x) function. get_at(X x) returns
 * value, that must be comparable by <=.
 *
 * @param optimizer a struct which defines an optimizer.; requires a get_at_(X)
 * function
 * @return si::speed
 */
template <typename X, typename Y>
si::speed golden_section_search_min_optimizer(X xlow, X xhigh, X err,
                                              Y optimizer) {
  // define d lambda expression: d = 0.5 * (sqrt(5) - 1) * (xhigh - xlow)
  auto d = [](X low, X high) { return 0.5 * (sqrt(5.0) - 1.0) * (high - low); };

  // let x1 = xlow + d and x2 = xhigh - d
  X x1 = xlow + d(xlow, xhigh);
  X x2 = xhigh - d(xlow, xhigh);

  // while xhigh - xlow > err:update x1, x2, xhigh, xlow
  while (xhigh - xlow > err) {
    if (optimizer.get_at(x1) <= optimizer.get_at(x2)) {
      xlow = x2;
      x2 = x1;
      x2 = x1;
      x1 = xlow + d(xlow, xhigh);
    } else {
      xhigh = x1;
      x1 = x2;
      x2 = xhigh - d(xlow, xhigh);
    }
  }

  // determine x as 0.5 * (xhigh + xlow)
  return 0.5 * (xhigh + xlow);
};

}  // namespace soro::runtime