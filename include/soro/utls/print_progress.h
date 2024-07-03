#pragma once

#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <source_location>
#include <thread>

#include "utl/logging.h"

#include "soro/base/soro_types.h"

namespace soro::utls {

constexpr auto const WIDTH = 30U;
constexpr auto const FILLED = "█";
constexpr auto const NOT_FILLED = "░";

inline std::string format_duration(std::chrono::milliseconds duration) {
  using namespace std::chrono;

  auto const h = duration_cast<hours>(duration);
  auto const m = duration_cast<minutes>(duration -= h);
  auto const s = duration_cast<seconds>(duration -= m);
  auto const ms = duration_cast<milliseconds>(duration -= s);

  return fmt::format("{:02}h {:02}m {:02}s {:03}ms", h.count(), m.count(),
                     s.count(), ms.count());
}

struct progress_bar {
  progress_bar() = default;
  progress_bar(std::string const& name, std::size_t const max)
      : name_{name},
        max_{max},
        current_{1},
        start_time_{std::chrono::system_clock::now()},
        end_time_{std::chrono::system_clock::now()} {}

  void increment(std::size_t const v = 1) {
    end_time_ = std::chrono::system_clock::now();
    current_ += v;
  }

  void print(std::ostream& out) const {
    using namespace std::chrono;

    out << "\x1b[K";  // clear line

    out << "[prog]"
        << "[" << utl::time() << "][" << name_ << "][";

    auto const percent =
        (static_cast<double>(current_) / static_cast<double>(max_));

    auto const filled =
        static_cast<uint32_t>(static_cast<double>(WIDTH) * percent);
    for (auto i = 0U; i < WIDTH; ++i) {
      out << (i < filled ? FILLED : NOT_FILLED);
    }

    out << "]" << fmt::format("({:.0f}%)", percent * 100.0);

    auto const duration = duration_cast<milliseconds>(end_time_ - start_time_);
    out << " " << format_duration(duration);

    out << std::endl;
  }

  std::string name_;
  std::size_t max_;

  std::size_t current_;
  std::chrono::time_point<std::chrono::system_clock> start_time_;
  std::chrono::time_point<std::chrono::system_clock> end_time_;
};

constexpr bool operator<(std::source_location const& l1,
                         std::source_location const& l2) {
  if (l1.line() != l2.line()) return l1.line() < l2.line();
  if (l1.column() != l2.column()) return l1.column() < l2.column();
  return l1.file_name() < l2.file_name();
}

struct threaded_location {
  threaded_location(std::source_location const& loc)
      : thread_id_{std::this_thread::get_id()}, loc_{loc} {}

  bool operator<(threaded_location const& other) const {
    if (thread_id_ != other.thread_id_) return thread_id_ < other.thread_id_;
    return loc_ < other.loc_;
  }

  std::thread::id thread_id_;
  std::source_location loc_;
};

template <typename Container>
constexpr void print_progress(
    std::string const& name, Container const& c,
    soro::size_t const increment_value = 1,
    std::source_location const sloc = std::source_location::current(),
    std::ostream& out = std::cerr) {

  static std::shared_mutex map;
  static std::mutex print;
  static std::map<threaded_location, progress_bar> bars;

  threaded_location const loc{sloc};

  map.lock_shared();
  auto const it = bars.find(loc);
  map.unlock_shared();

  if (it == std::end(bars)) {
    std::lock_guard write_lock(map);
    bars[loc] = progress_bar(name, c.size());
  } else {
    std::lock_guard write_lock(map);
    it->second.increment(increment_value);
  }

  std::lock_guard print_lock(print);
  out << "\x1b[" << bars.size() << "A";
  for (auto const& [_, bar] : bars) bar.print(out);
}

}  // namespace soro::utls
