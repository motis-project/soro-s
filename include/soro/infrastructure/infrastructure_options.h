#pragma once

#include "cista/reflection/comparable.h"

#include "soro/infrastructure/exclusion/exclusion.h"
#include "soro/infrastructure/exclusion/exclusion_graph.h"
#include "soro/infrastructure/interlocking/interlocking.h"

#include <filesystem>

namespace soro::infra {

template <typename Tag>
struct option {
  option(bool const b) : b_{b} {}  // NOLINT
  operator bool() const { return b_; }  // NOLINT
  bool b_;
};

struct layout_tag;
struct exclusion_sets_tag;

struct infrastructure_options {
  CISTA_COMPARABLE()

  option<interlocking> interlocking_{true};
  option<exclusion> exclusions_{true};
  option<layout_tag> layout_{true};
  option<exclusion_sets_tag> exclusion_sets_{true};

  std::filesystem::path infrastructure_path_{""};
  std::filesystem::path gps_coord_path_{""};
};

inline infrastructure_options make_infra_opts(
    std::filesystem::path const& infrastructure_path,
    std::filesystem::path const& coord_path) {
  return infrastructure_options{.infrastructure_path_ = infrastructure_path,
                                .gps_coord_path_ = coord_path};
}

}  // namespace soro::infra