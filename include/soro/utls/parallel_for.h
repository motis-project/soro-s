#pragma once

#include "utl/parallel_for.h"

namespace soro::utls {

template <typename Container, typename Fun>
inline utl::errors_t parallel_for(
    Container const& jobs, Fun&& func,
    [[maybe_unused]] utl::parallel_error_strategy const err_strat =
        utl::parallel_error_strategy::QUIT_EXEC) {
#if defined(__EMSCRIPTEN__)
  std::for_each(std::cbegin(jobs), std::cend(jobs), func);
  return {};
#else
  return utl::parallel_for(jobs, func, err_strat);
#endif
}

template <typename Container, typename Fun>
inline utl::errors_t parallel_for(
    [[maybe_unused]] std::string const& desc, Container const& jobs,
    [[maybe_unused]] size_t const mod, Fun func,
    [[maybe_unused]] utl::parallel_error_strategy const err_strat =
        utl::parallel_error_strategy::QUIT_EXEC) {
#if defined(__EMSCRIPTEN__)
  // TODO(julian) enable threads for emscripten
  std::for_each(std::cbegin(jobs), std::cend(jobs), func);
  return {};
#else
  return utl::parallel_for(desc, jobs, mod, func, err_strat);
#endif
}

template <typename Container = size_t, typename Fun>
inline utl::errors_t parallel_for(
    size_t const job_count, Fun func,
    [[maybe_unused]] utl::parallel_error_strategy const err_strat =
        utl::parallel_error_strategy::QUIT_EXEC) {
#if defined(__EMSCRIPTEN__)
  // TODO(julian) enable threads for emscripten
  for (size_t idx = 0; idx < job_count; ++idx) {
    func(idx);
  }

  return {};
#else
  return utl::parallel_for_run(job_count, func, err_strat);
#endif
}

}  // namespace soro::utls
