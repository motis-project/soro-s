#pragma once

#include <concepts>
#include <algorithm>
#include <atomic>
#include <exception>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "soro/base/soro_types.h"

#include "soro/utls/narrow.h"

namespace soro::utls {

namespace detail {

using thread_count_t = decltype(std::thread::hardware_concurrency());
const thread_count_t default_thread_count = std::thread::hardware_concurrency();

template <typename WorkIterable, typename DoWork,
          typename Input = decltype(*std::begin(std::declval<WorkIterable>()))>
concept object_work = requires { requires std::invocable<DoWork, Input>; };

template <typename WorkIterable, typename DoWork,
          typename Input = decltype(std::declval<WorkIterable>().size())>
concept index_work = requires { requires std::invocable<DoWork, Input>; };

enum class parallel_error_strategy { CONTINUE_EXEC, QUIT_EXEC };

using errors_t = std::vector<std::pair<size_t, std::exception_ptr>>;

enum class combine_execution_policy { SEQUENTIAL, PARALLEL };

template <typename DoWork>
inline errors_t parallel_for(std::integral auto const job_count,
                             DoWork&& do_work,
                             thread_count_t const thread_count,
                             parallel_error_strategy const err_strat =
                                 parallel_error_strategy::QUIT_EXEC) {

  errors_t errors;
  std::mutex errors_mutex;
  std::atomic<std::remove_const_t<decltype(job_count)>> counter(0);
  std::atomic<bool> quit{false};
  std::vector<std::thread> threads;

  for (thread_count_t i{0}; i < thread_count; ++i) {
    threads.emplace_back([&]() {
      while (!quit) {
        auto const idx = counter.fetch_add(1);
        if (idx >= job_count) {
          break;
        }

        try {
          do_work(idx);
        } catch (...) {
          std::lock_guard<std::mutex> const lock{errors_mutex};
          errors.emplace_back(std::pair{i, std::current_exception()});
          if (err_strat == parallel_error_strategy::QUIT_EXEC) {
            quit = true;
            break;
          }
        }
      }
    });
  }

  std::for_each(begin(threads), end(threads), [](auto& t) { t.join(); });

  if (err_strat == parallel_error_strategy::QUIT_EXEC && !errors.empty()) {
    std::rethrow_exception(errors.front().second);
  }

  return errors;
}

template <typename EndResult, typename PartialResult, typename WorkIterable,
          typename DoWork, typename WorkItemAccessor, typename CombineResults>
EndResult parallel_for(WorkIterable&& work, DoWork&& do_work,
                       CombineResults&& combine_results,
                       WorkItemAccessor&& access,
                       thread_count_t const thread_count,
                       combine_execution_policy combine_exec =
                           combine_execution_policy::SEQUENTIAL) {

  // for now the combine exec is always sequential
  std::ignore = combine_exec;

  std::vector<PartialResult> partial_results(work.size());

  parallel_for(
      work.size(),
      [&](auto&& work_id) {
        partial_results[work_id] =
            do_work(access(utls::narrow<soro::size_t>(work_id), work));
      },
      thread_count);

  EndResult result;
  for (auto& partial_result : partial_results) {
    combine_results(result, partial_result);
  }

  return result;
}

}  // namespace detail

// overload for when DoWork expects the object given by WorkIterable
template <typename EndResult, typename WorkIterable, typename DoWork,
          typename CombineResults>
  requires detail::object_work<WorkIterable, DoWork>
[[nodiscard]] constexpr EndResult parallel_for(
    WorkIterable&& work, DoWork&& do_work, CombineResults&& combine_results,
    detail::thread_count_t const thread_count = detail::default_thread_count) {

  using partial_result_t =
      std::invoke_result_t<DoWork,
                           decltype(*std::begin(std::declval<WorkIterable>()))>;

  static constexpr auto const access =
      [](auto&& work_id, auto&& work_in) -> auto& { return work_in[work_id]; };

  return detail::parallel_for<EndResult, partial_result_t>(
      work, do_work, combine_results, access, thread_count);
}

// overload for when DoWork expects a job id determined by the .size() of
// WorkIterable
template <typename EndResult, typename WorkIterable, typename DoWork,
          typename CombineResults>
  requires detail::index_work<WorkIterable, DoWork>
[[nodiscard]] constexpr EndResult parallel_for(
    WorkIterable&& work, DoWork&& do_work, CombineResults&& combine_results,
    detail::thread_count_t const thread_count = detail::default_thread_count) {

  using partial_result_t =
      std::invoke_result_t<DoWork,
                           decltype(std::declval<WorkIterable>().size())>;

  static constexpr auto const access = [](auto&& work_id, auto&&) {
    return work_id;
  };

  return detail::parallel_for<EndResult, partial_result_t>(
      work, do_work, combine_results, access, thread_count);
}

template <typename DoWork>
constexpr void parallel_for(
    std::integral auto job_count, DoWork&& do_work,
    detail::thread_count_t const thread_count = detail::default_thread_count) {
  detail::parallel_for(job_count, do_work, thread_count);
}

}  // namespace soro::utls