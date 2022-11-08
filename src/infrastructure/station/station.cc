#include "soro/infrastructure/station/station.h"

#include "fmt/format.h"

#include "utl/erase_duplicates.h"
#include "utl/pipes.h"

namespace soro::infra {

soro::vector<station::ptr> station::neighbours() const {
  auto neighbours = utl::all(borders_) |
                    utl::transform([](auto&& b) { return b.neighbour_; }) |
                    utl::emplace_back<soro::vector<station::ptr>>();
  utl::erase_duplicates(neighbours);
  return neighbours;
}

}  // namespace soro::infra