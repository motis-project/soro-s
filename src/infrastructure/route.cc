#include "soro/infrastructure/route.h"

#include "soro/utls/std_wrapper/std_wrapper.h"

namespace soro::infra {

std::pair<node::idx, node::idx> fast_forward_indices(route const& r,
                                                     node::idx const ff_to) {
  std::pair<node::idx, node::idx> indices{
      r.omitted_nodes_.empty() ? node::INVALID_IDX : 0,
      r.extra_speed_limits_.empty() ? node::INVALID_IDX : 0};

  if (ff_to == 0) {
    return indices;
  }

  if (!r.omitted_nodes_.empty()) {
    auto const omit_idx = utls::find_if_position(
        r.omitted_nodes_,
        [&ff_to](node::idx const omitted_idx) { return omitted_idx >= ff_to; });
    indices.first = static_cast<node::idx>(omit_idx);
  }

  if (r.extra_speed_limits_.empty()) {
    return indices;
  }

  node::idx spl_idx = 0;
  for (node::idx n_idx = 0; n_idx < ff_to; ++n_idx) {
    auto const& spl = r.extra_speed_limits_[spl_idx];
    if (spl.node_ == r.nodes_[n_idx]) {
      ++spl_idx;
    }

    if (spl_idx == r.extra_speed_limits_.size()) {
      break;
    }
  }

  indices.second = spl_idx;

  return indices;
}

utls::recursive_generator<route_node> route::from_to(
    node::idx const from, node::idx const to, skip_omitted const skip) const {
  route_node result;
  result.node_idx_ = from;

  auto [omit_idx, spl_idx] = fast_forward_indices(*this, from);

  for (; result.node_idx_ < to; ++result.node_idx_) {
    result.node_ = nodes_[result.node_idx_];

    result.omitted_ = omit_idx != node::INVALID_IDX &&
                      omitted_nodes_[omit_idx] == result.node_idx_;

    if (result.omitted_) {
      ++omit_idx;
    } else {
      bool const extra_spl =
          spl_idx != node::INVALID_IDX &&
          extra_speed_limits_[spl_idx].node_->id_ == result.node_->id_;

      result.extra_spl_ = extra_spl ? &extra_speed_limits_[spl_idx++]
                                    : std::optional<speed_limit::ptr>{};
    }

    if (!result.omitted_ || static_cast<bool>(skip)) {
      co_yield result;
    }
  }
}

utls::recursive_generator<route_node> route::entire(
    skip_omitted const skip) const {
  return from_to(0, size(), skip);
}

node::idx route::size() const noexcept {
  assert(nodes_.size() < std::numeric_limits<node::idx>::max());
  return static_cast<node::idx>(nodes_.size());
}

}  // namespace soro::infra