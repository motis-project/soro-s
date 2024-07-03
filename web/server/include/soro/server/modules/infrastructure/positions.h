#pragma once

#include <filesystem>
#include <unordered_map>

#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/layout.h"

#include "soro/utls/coordinates/gps.h"

namespace soro::server {

struct positions {
  soro::vector_map<infra::station::id, utls::gps> stations_;
  soro::vector_map<infra::element::id, utls::gps> elements_;
  soro::vector_map<infra::node::id, utls::gps> nodes_;
};

positions get_positions(
    infra::infrastructure const& infra,
    soro::map<infra::station::ds100, utls::gps> const& station_pos);

}  // namespace soro::server
