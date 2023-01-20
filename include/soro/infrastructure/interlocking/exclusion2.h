#pragma once

#include "soro/infrastructure/infrastructure.h"

namespace soro::infra {

struct exclusion_set {
  using id = uint32_t;
  using offset_t = soro::size_t;
  using bitvec_t = cista::raw::bitvec;

  static constexpr auto INVALID_OFFSET = std::numeric_limits<offset_t>::max();

  exclusion_set() = default;

  template <typename T>
  exclusion_set(std::vector<T> const& sorted_ids) {
    utls::sassert(utls::is_sorted(sorted_ids), "IDs not sorted.");

    if (sorted_ids.empty()) {
      return;
    }

    first_ = sorted_ids.front();
    last_ = sorted_ids.back() + 1;
    bits_.resize(static_cast<uint32_t>(last_ - first_));

    for (auto const id : sorted_ids) {
      bits_.set(static_cast<uint32_t>(id - first_));
    }
  }

  void set_first(offset_t const new_first) {
    if (first_ == new_first) {
      return;
    }

    utls::sassert(new_first < first_);

    this->bits_.resize(bits_.size() + first_ - new_first);
    this->bits_ <<= first_ - new_first;

    this->first_ = new_first;
  }

  void set_last(offset_t const new_last) {
    if (last_ == new_last) {
      return;
    }

    utls::sassert(new_last > last_);
    this->bits_.resize(bits_.size() + new_last - last_);
    this->last_ = new_last;
  }

  bool contains(exclusion_set const& other) const {
    if (this->empty()) {
      return false;
    }

    if (other.empty()) {
      return true;
    }

    utls::sassert(first_ == other.first_ && last_ == other.last_ &&
                  bits_.size() == other.bits_.size());

    for (auto i = 0U; i < bits_.size(); ++i) {
      if (!this->bits_[i] && other.bits_[i]) {
        return false;
      }
    }

    return true;
  }

  bool operator==(exclusion_set const& o) const noexcept = default;

  void clear() {
    first_ = INVALID_OFFSET;
    last_ = INVALID_OFFSET;
    bits_ = {};
  }

  bool empty() const {
    return first_ == INVALID_OFFSET && last_ == INVALID_OFFSET && bits_.empty();
  }

  offset_t first_{INVALID_OFFSET};
  offset_t last_{INVALID_OFFSET};

  bitvec_t bits_{};
};

soro::vector<exclusion_set> get_interlocking_exclusion_sets(
    infrastructure const& infra);

}  // namespace soro::infra
