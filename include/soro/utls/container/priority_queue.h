#pragma once

#include <algorithm>
#include <vector>

namespace soro::utls {

template <typename T, typename Comparator>
struct priority_queue {
  priority_queue(Comparator comp_fn) : comp_fn_{comp_fn} {}

  T const& top() const { return mem_.front(); }

  void pop() {
    std::pop_heap(std::begin(mem_), std::end(mem_), comp_fn_);
    mem_.pop_back();
  }

  bool empty() const { return mem_.empty(); }

  template <typename... Args>
  void emplace(Args&&... args) {
    mem_.emplace_back(std::forward<Args...>(args...));
    std::push_heap(std::begin(mem_), std::end(mem_), comp_fn_);
  }

  auto begin() const { return std::cbegin(mem_); }
  auto end() const { return std::cend(mem_); }

  Comparator comp_fn_;
  std::vector<T> mem_;
};

}  // namespace soro::utls
