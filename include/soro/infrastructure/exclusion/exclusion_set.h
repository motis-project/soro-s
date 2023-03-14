#pragma once

#include "soro/base/soro_types.h"

#include "cista/containers/bitvec.h"

#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/std_wrapper.h"

namespace soro::infra {

struct exclusion_set {
  using value_type = uint32_t;
  using id = uint32_t;
  using bitvec_t = soro::data::bitvec;

  static constexpr auto INVALID_ID = std::numeric_limits<id>::max();
  static constexpr auto INVALID_OFFSET = std::numeric_limits<value_type>::max();

  using optional_id = soro::optional<id>;
  using ids = soro::vector<id>;

  struct iterator {
    using iterator_category = typename std::input_iterator_tag;
    using value_type = exclusion_set::value_type;
    using difference_type = value_type;
    using pointer = value_type*;
    using reference = value_type;

    iterator(exclusion_set const* const set, bitvec_t::size_type const idx)
        : set_{set}, idx_{idx} {
      utls::expect(idx_ <= set_->bits_.size(),
                   "initializing idx larger than size");
    }

    iterator operator+(std::size_t n) {
      auto result = *this;

      while (n != 0) {
        ++result;
        --n;
      }

      return result;
    }

    iterator& operator++() {
      ++idx_;
      while (idx_ < set_->bits_.size() && !set_->bits_[idx_]) {
        ++idx_;
      }

      return *this;
    }

    bool operator==(iterator const& other) const = default;
    bool operator!=(iterator const& other) const = default;

    value_type operator*() const { return set_->first_ + idx_; }
    pointer operator->() = delete;

  private:
    exclusion_set const* set_{nullptr};
    bitvec_t::size_type idx_{std::numeric_limits<bitvec_t::size_type>::max()};
  };

  auto begin() const { return iterator{this, first_bit_set_ - first_}; }
  auto end() const { return iterator{this, bits_.size()}; }

  soro::vector<value_type> expanded_set() const;

  bool operator[](value_type const idx) const;

  void set(value_type const idx);

  exclusion_set& operator-=(exclusion_set const& other);
  exclusion_set& operator|=(exclusion_set const& other);

  exclusion_set operator-(exclusion_set const& other) const;
  exclusion_set operator|(exclusion_set const& other) const;

  std::partial_ordering compare(exclusion_set const& other) const;

  bool contains(exclusion_set const& other) const;

  bool operator==(exclusion_set const& o) const noexcept = default;

  value_type size() const;
  std::size_t count() const;

  void clear();

  bool any() const;
  bool empty() const;
  bool ok() const;

  id id_{INVALID_ID};

  value_type first_{INVALID_OFFSET};
  value_type last_{INVALID_OFFSET};

  value_type first_bit_set_{INVALID_OFFSET};
  value_type last_bit_set_{INVALID_OFFSET};

  bitvec_t bits_{};
};

exclusion_set make_exclusion_set(
    exclusion_set::id const id,
    soro::vector<exclusion_set::value_type> const& sorted_ids);

exclusion_set make_exclusion_set(
    soro::vector<exclusion_set::value_type> const& sorted_ids);

}  // namespace soro::infra
