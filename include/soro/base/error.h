#pragma once

#include <map>
#include <system_error>
#include <type_traits>

#include "utl/logging.h"

#include "soro/utls/result.h"
#include "soro/utls/sassert.h"

namespace soro::error {

struct stats {
  explicit stats(std::string_view const name) : name_{name} {}

  template <typename T>
  void count(utls::result<T> const& result) {
    utls::expect(!result, "error counting a valid result");
    count(result.error());
  }

  void count(std::error_code const e) {
    std::lock_guard const lock{mutex_};
    ++errors_[e];
  }

  void report() const {
    if (errors_.empty()) {
      return;
    }

    uLOG(utl::info) << "error report for [" << name_ << "]:";
    for (auto const& [e, count] : errors_) {
      uLOG(utl::info) << e.message() << ": " << count;
    }
  }

  std::string_view name_{"give me name"};
  std::mutex mutex_;
  std::map<std::error_code, std::size_t> errors_;
};

}  // namespace soro::error
