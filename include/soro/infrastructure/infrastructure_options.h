#pragma once

#include "cista/reflection/comparable.h"

#include <filesystem>

namespace soro::infra {

// Parse an infrastructure from the following:
//   - ISS 'Index.xml', which references all other ISS-Xid_ML files
//   - ISS 'Spurplanbtrs.xml', which only contains station information
//   - ISS 'infra.base_infrastructure', file, which is a .tar.zst file
//   containing ISS-XML files
//   - ISS 'example/', a folder which contains an 'Index.xml'

struct infrastructure_options {
  CISTA_COMPARABLE();

  bool determine_interlocking_{true};
  bool determine_conflicts_{false};
  bool determine_layout_{false};

  std::filesystem::path gps_coord_path_{""};
  std::filesystem::path infrastructure_path_{""};
};

}  // namespace soro::infra