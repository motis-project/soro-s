#pragma once

#include <iosfwd>

#include "rapid/network.h"

namespace rapid {

struct printable_path {
  network const& net_;
  std::vector<edge*> path_;
};

std::ostream& operator<<(std::ostream&, printable_path const&);

}  // namespace rapid