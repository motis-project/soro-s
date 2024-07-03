#include "soro/infrastructure/exclusion/exclusion_set.h"

#include <compare>
#include <cstddef>
#include <algorithm>
#include <type_traits>

#include "soro/base/soro_types.h"

#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/is_sorted.h"

#include "soro/infrastructure/interlocking/interlocking_route.h"

namespace soro::infra {

void compact(exclusion_set& set) {
  // compact front
  auto const old_first = set.first_;
  set.first_ = (set.first_bit_set_ / exclusion_set::bitvec_t::bits_per_block) *
               exclusion_set::bitvec_t::bits_per_block;
  set.bits_ >>= (set.first_ - old_first);
  set.last_ += (set.first_ - old_first);

  // compact back
  auto const extra_blocks =
      (set.last_ - set.last_bit_set_) / exclusion_set::bitvec_t::bits_per_block;
  auto const extra_bits =
      extra_blocks * exclusion_set::bitvec_t::bits_per_block;
  set.bits_.resize(set.bits_.size() - extra_bits);
  set.last_ -= extra_bits;
}

exclusion_set::value_type get_first_set(exclusion_set const& set) {
  exclusion_set::value_type fs = 0;
  for (; !set.bits_[fs] && fs < set.bits_.size(); ++fs)
    ;

  return fs == set.bits_.size() ? exclusion_set::INVALID_OFFSET
                                : fs + set.first_;
}

exclusion_set::value_type get_last_set(exclusion_set const& set) {
  exclusion_set::value_type ls = set.bits_.size() - 1;
  // we need wrapping
  static_assert(std::is_unsigned_v<exclusion_set::value_type>);
  for (; ls != exclusion_set::INVALID_OFFSET && !set.bits_[ls]; --ls)
    ;

  return ls == exclusion_set::INVALID_OFFSET ? ls : ls + set.first_;
}

void lower_first(exclusion_set* set,
                 exclusion_set::value_type const new_first) {
  utls::expect(set->first_ > new_first, "New first le than current.");
  utls::expect(new_first % exclusion_set::bitvec_t::bits_per_block == 0,
               "New first not divisible by bits per block.");

  auto const dist = set->first_ - new_first;
  set->bits_.resize(set->bits_.size() + dist);
  set->bits_ <<= dist;
  set->first_ = new_first;
}

void increase_last(exclusion_set* set,
                   exclusion_set::value_type const new_last) {
  utls::expect(set->last_ < new_last, "New last ge than current.");
  utls::expect((new_last + 1) % exclusion_set::bitvec_t::bits_per_block == 0,
               "New last not divisible by bits per block.");

  auto const dist = new_last - set->last_;
  set->bits_.resize(set->bits_.size() + dist);
  set->last_ = new_last;
}

bool contains_impl(exclusion_set const& a, exclusion_set const& b) {
  utls::expect(a.first_ <= b.first_ && a.last_ >= b.last_,
               "b is not fully contained in a, it can't be a subset");

  auto constexpr bits_per_block = exclusion_set::bitvec_t::bits_per_block;

  // make sure we don't need to sanitize the last block
  utls::sassert(a.bits_.size() % bits_per_block == 0,
                "Would have to sanitize last block of a!");
  utls::sassert(b.bits_.size() % bits_per_block == 0,
                "Would have to sanitize last block of b");

  auto const diff_in_bits = b.first_ - a.first_;
  utls::sassert(diff_in_bits % bits_per_block == 0,
                "Diff in bits not multiple of bits per block");
  auto const diff_in_blocks = diff_in_bits / bits_per_block;

  // make sure we do not go out of bounds in the following loop
  utls::sassert(
      b.bits_.blocks_.size() - 1 + diff_in_blocks < a.bits_.blocks_.size(),
      "Would go out of bounds in the following loop with {} blocks in a and {} "
      "blocks in b",
      a.bits_.blocks_.size(), b.bits_.blocks_.size());

  for (auto i = 0U; i < b.bits_.blocks_.size(); ++i) {
    if ((b.bits_.blocks_[i] | a.bits_.blocks_[i + diff_in_blocks]) !=
        a.bits_.blocks_[i + diff_in_blocks]) {
      return false;
    }
  }

  return true;
}

std::partial_ordering contains_impl_same_size(exclusion_set const& a,
                                              exclusion_set const& b) {
  utls::expect(a.first_ == b.first_ && a.last_ == b.last_,
               "a and b must be the same size");

  auto constexpr bits_per_block = exclusion_set::bitvec_t::bits_per_block;

  // make sure we don't need to sanitize the last block
  utls::sassert(a.bits_.size() % bits_per_block == 0,
                "Would have to sanitize last block of a!");
  utls::sassert(b.bits_.size() % bits_per_block == 0,
                "Would have to sanitize last block of b");

  // make sure we do not go out of bounds in the following loop
  utls::sassert(a.bits_.blocks_.size() == b.bits_.blocks_.size());

  bool a_is_sub = true;
  bool b_is_sub = true;

  for (auto i = 0U; i < a.bits_.blocks_.size(); ++i) {
    a_is_sub &= (a.bits_.blocks_[i] | b.bits_.blocks_[i]) == b.bits_.blocks_[i];
    b_is_sub &= (b.bits_.blocks_[i] | a.bits_.blocks_[i]) == a.bits_.blocks_[i];

    if (!a_is_sub && !b_is_sub) {
      break;
    }
  }

  if (a_is_sub) {
    return b_is_sub ? std::partial_ordering::equivalent
                    : std::partial_ordering::less;
  } else {  // 'a' is not a subset
    return b_is_sub ? std::partial_ordering::greater
                    : std::partial_ordering::unordered;
  }
}

exclusion_set::iterator::iterator(exclusion_set const* const set,
                                  exclusion_set::bitvec_t::size_type const idx)
    : set_{set}, idx_{idx} {
  utls::expect(idx_ <= set_->bits_.size(), "initializing idx larger than size");
}

exclusion_set::iterator exclusion_set::iterator::operator+(std::size_t n) {
  auto result = *this;

  while (n != 0) {
    ++result;
    --n;
  }

  return result;
}

exclusion_set::iterator& exclusion_set::iterator::operator++() {
  ++idx_;
  while (idx_ < set_->bits_.size() && !set_->bits_[idx_]) {
    ++idx_;
  }

  return *this;
}

exclusion_set::iterator::value_type exclusion_set::iterator::operator*() const {
  return exclusion_set::iterator::value_type{set_->first_ + idx_};
}

soro::vector<interlocking_route::id> exclusion_set::expanded_set() const {
  soro::vector<interlocking_route::id> result;

  for (auto i = 0U; i < bits_.size(); ++i) {
    if (bits_[i]) {
      result.emplace_back(first_ + i);
    }
  }

  return result;
}

bool exclusion_set::any() const { return this->bits_.any(); }

bool exclusion_set::operator[](interlocking_route::id const ir_id) const {
  auto const idx = as_val(ir_id);

  if (idx < first_bit_set_ || idx > last_bit_set_) {
    return false;
  }

  return this->bits_[idx - first_];
}

void exclusion_set::set(interlocking_route::id const ir_id) {
  auto const idx = as_val(ir_id);

  if (idx < first_) {
    lower_first(this,
                (idx / bitvec_t::bits_per_block) * bitvec_t::bits_per_block);
  } else if (idx > last_) {
    increase_last(this, (((idx / bitvec_t::bits_per_block) + 1) *
                         bitvec_t::bits_per_block) -
                            1);
  }

  this->bits_.set(idx - first_);

  first_bit_set_ = std::min(idx, first_bit_set_);
  last_bit_set_ = std::max(idx, last_bit_set_);

  utls::sassert(ok());
}

exclusion_set& exclusion_set::operator-=(exclusion_set const& other) {
  if (this == &other) {
    this->clear();
    return *this;
  }

  if (this->empty() || other.empty()) {
    return *this;
  }

  // test intersection and return early
  if (!(this->last_bit_set_ >= other.first_bit_set_ &&
        other.last_bit_set_ >= this->first_bit_set_)) {
    return *this;
  }

  auto const this_smaller = this->first_ <= other.first_;

  auto const bit_diff =
      this_smaller ? other.first_ - this->first_ : this->first_ - other.first_;

  utls::sassert(bit_diff % bitvec_t::bits_per_block == 0,
                "bit diff not divisible by block size");

  auto const diff = bit_diff / bitvec_t::bits_per_block;

  auto const to = this_smaller ? std::min(this->bits_.blocks_.size() - diff,
                                          other.bits_.blocks_.size())
                               : std::min(this->bits_.blocks_.size(),
                                          other.bits_.blocks_.size() - diff);

  for (auto i = 0U; i < to; ++i) {
    this_smaller ? bits_.blocks_[i + diff] &= ~other.bits_.blocks_[i]
                 : bits_.blocks_[i] &= ~other.bits_.blocks_[i + diff];
  }

  if (!bits_[first_bit_set_ - first_]) {
    first_bit_set_ = get_first_set(*this);
  }

  if (first_bit_set_ == INVALID_OFFSET) {
    this->clear();
    return *this;
  }

  if (!bits_[last_bit_set_ - first_]) {
    last_bit_set_ = get_last_set(*this);
  }

  compact(*this);

  utls::ensure(ok(), "exclusion set invariant violated");

  return *this;
}

exclusion_set& exclusion_set::operator|=(exclusion_set const& other) {
  if (this == &other || other.empty()) {
    return *this;
  }

  if (this->empty()) {
    *this = other;
  }

  if (this->first_ > other.first_) {
    lower_first(this, other.first_);
  }

  if (this->last_ < other.last_) {
    increase_last(this, other.last_);
  }

  auto const diff_in_bits = other.first_ - this->first_;
  utls::sassert(diff_in_bits % bitvec_t::bits_per_block == 0);
  auto const diff_in_blocks = diff_in_bits / bitvec_t::bits_per_block;

  for (auto i = 0U; i < other.bits_.blocks_.size(); ++i) {
    this->bits_.blocks_[i + diff_in_blocks] |= other.bits_.blocks_[i];
  }

  this->first_bit_set_ = std::min(this->first_bit_set_, other.first_bit_set_);
  this->last_bit_set_ = std::max(this->last_bit_set_, other.last_bit_set_);

  utls::ensure(ok(), "exclusion set invariant violated");

  return *this;
}

exclusion_set exclusion_set::operator-(exclusion_set const& other) const {
  auto result = *this;
  result -= other;
  return result;
}

exclusion_set exclusion_set::operator|(exclusion_set const& other) const {
  auto result = *this;
  result |= other;
  return result;
}

std::partial_ordering exclusion_set::compare(exclusion_set const& other) const {
  if (this->empty() && !other.empty()) {
    return std::partial_ordering::less;
  }

  if (other.empty()) {
    return std::partial_ordering::greater;
  }

  if (this == &other) {
    return std::partial_ordering::equivalent;
  }

  if (this->first_ == other.first_ && this->last_ == other.last_) {
    return contains_impl_same_size(*this, other);
  }

  if (this->first_bit_set_ <= other.first_bit_set_ &&
      this->last_bit_set_ >= other.last_bit_set_) {
    return contains_impl(*this, other) ? std::partial_ordering::greater
                                       : std::partial_ordering::unordered;
  }

  if (this->first_bit_set_ >= other.first_bit_set_ &&
      this->last_bit_set_ <= other.last_bit_set_) {
    return contains_impl(other, *this) ? std::partial_ordering::less
                                       : std::partial_ordering::unordered;
  }

  return std::partial_ordering::unordered;
}

bool exclusion_set::contains(exclusion_set const& other) const {
  return std::is_gteq(this->compare(other));
}

exclusion_set::value_type exclusion_set::size() const { return last_ - first_; }

std::size_t exclusion_set::count() const { return bits_.count(); }

void exclusion_set::clear() {
  first_ = INVALID_OFFSET;
  last_ = INVALID_OFFSET;

  first_bit_set_ = INVALID_OFFSET;
  last_bit_set_ = INVALID_OFFSET;

  bits_ = {};
}

bool exclusion_set::empty() const {
  return first_ == INVALID_OFFSET && last_ == INVALID_OFFSET &&
         first_bit_set_ == INVALID_OFFSET && last_bit_set_ == INVALID_OFFSET &&
         bits_.empty();
}

bool exclusion_set::ok() const {
  if (this->empty()) {
    return true;
  }

  auto const no_invalid = first_ != INVALID_OFFSET && last_ != INVALID_OFFSET &&
                          first_bit_set_ != INVALID_OFFSET &&
                          last_bit_set_ != INVALID_OFFSET && !bits_.empty();

  if (!no_invalid) {
    utls::sassert(false);
    return false;
  }

  auto const first_correct = this->first_ <= this->first_bit_set_;
  auto const last_correct = this->last_ >= this->last_bit_set_;

  if (!first_correct || !last_correct) {
    utls::sassert(false);
    return false;
  }

  auto const first_before_last = first_ < last_;

  if (!first_before_last) {
    utls::sassert(false);
    return false;
  }

  auto const no_unnecessary_first_blocks =
      first_bit_set_ <= first_ + bitvec_t::bits_per_block;
  auto const no_unnecessary_last_blocks =
      last_bit_set_ >= last_ - (bitvec_t::bits_per_block - 1);

  if (!no_unnecessary_first_blocks || !no_unnecessary_last_blocks) {
    utls::sassert(false);
    return false;
  }

  auto const dist = this->last_bit_set_ - this->first_bit_set_ + 1;
  auto const bits_correctly_sized =
      this->bits_.size() >= (dist / bitvec_t::bits_per_block) + 1;

  if (!bits_correctly_sized) {
    utls::sassert(false);
    return false;
  }

  auto const first_bit_is_set = bits_[first_bit_set_ - first_];
  auto const last_bit_is_set = bits_[last_bit_set_ - first_];

  if (!first_bit_is_set || !last_bit_is_set) {
    utls::sassert(false);
    return false;
  }

  value_type first_set = INVALID_OFFSET;
  value_type last_set = INVALID_OFFSET;
  for (auto idx = 0U; idx < bits_.size(); ++idx) {

    if (first_set == INVALID_OFFSET && bits_[idx]) {
      first_set = first_ + idx;
    }

    if (bits_[idx]) {
      last_set = first_ + idx;
    }
  }

  if (first_bit_set_ != first_set || last_bit_set_ != last_set) {
    utls::sassert(false);
    return false;
  }

  return true;
}

exclusion_set make_exclusion_set(
    exclusion_set::id const id,
    soro::vector<interlocking_route::id> const& sorted_ids) {
  utls::expect(utls::is_sorted(sorted_ids), "IDs not sorted.");

  if (sorted_ids.empty()) {
    exclusion_set result;
    result.id_ = id;
    return result;
  }

  exclusion_set es;
  es.id_ = id;

  es.first_bit_set_ = as_val(sorted_ids.front());
  es.last_bit_set_ = as_val(sorted_ids.back());

  es.first_ = (es.first_bit_set_ / exclusion_set::bitvec_t::bits_per_block) *
              exclusion_set::bitvec_t::bits_per_block;
  es.last_ =
      (((es.last_bit_set_ / exclusion_set::bitvec_t::bits_per_block) + 1) *
       exclusion_set::bitvec_t::bits_per_block) -
      1;

  es.bits_.resize(static_cast<exclusion_set::bitvec_t::size_type>(
      es.last_ - es.first_ + 1));

  for (auto const ir_id : sorted_ids) {
    es.bits_.set(
        static_cast<exclusion_set::bitvec_t::size_type>(ir_id - es.first_));
  }

  utls::ensure(es.ok(), "exclusion set invariant violated");

  return es;
}

exclusion_set make_exclusion_set(
    soro::vector<interlocking_route::id> const& sorted_ids) {
  return make_exclusion_set(exclusion_set::INVALID_ID, sorted_ids);
}

}  // namespace soro::infra
