#include "soro/server/modules/tiles/osm_export/osm_export.h"

#include <cstddef>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "utl/enumerate.h"
#include "utl/pairwise.h"
#include "utl/timer.h"

#include "soro/base/soro_types.h"

#include "soro/utls/coordinates/gps.h"
#include "soro/utls/sassert.h"

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/node.h"
#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/kilometrage.h"
#include "soro/infrastructure/station/station.h"

#include "soro/server/modules/infrastructure/positions.h"

namespace soro::server {

using namespace soro::infra;

struct osm {
  using tags = std::vector<std::pair<std::string, std::string>>;

  struct node {
    using id = soro::strong<std::size_t, struct _osm_node_id>;

    id id_{id::invalid()};
    utls::gps position_;

    tags tags_;
  };

  struct way {
    using id = soro::strong<std::size_t, struct _osm_way_id>;

    id id_{id::invalid()};

    std::vector<node::id> nodes_;

    tags tags_;
  };

  struct relation {
    using id = soro::strong<std::size_t, struct _osm_way_id>;

    id id_{id::invalid()};
  };

  bool ok() const {
    auto const are_consecutive = [](auto&& v) {
      return utls::all_of(utl::pairwise(v), [](auto&& p) {
        return std::get<0>(p).id_ + 1 == std::get<1>(p).id_;
      });
    };

    if (!nodes_.empty()) {
      auto const nodes_are_consecutive = are_consecutive(nodes_);
      auto const first = nodes_.back().id_ + 1 == nodes_.size();

      auto const nodes_are_ok = nodes_are_consecutive && first;

      utls::sassert(nodes_are_ok);
      if (!nodes_are_ok) return false;
    }

    if (!ways_.empty()) {
      auto const ways_are_consecutive = are_consecutive(ways_);
      auto const second = ways_.back().id_ + 1 == nodes_.size() + ways_.size();

      auto const ways_are_ok = ways_are_consecutive && second;

      utls::sassert(ways_are_ok);
      if (!ways_are_ok) return false;
    }

    if (!relations_.empty()) {
      auto const relations_are_consecutive = are_consecutive(relations_);
      auto const third = relations_.back().id_ + 1 ==
                         nodes_.size() + ways_.size() + relations_.size();

      auto const relations_are_ok = relations_are_consecutive && third;

      utls::sassert(relations_are_ok);
      if (!relations_are_ok) return false;
    }

    return true;
  }

  pugi::xml_document to_xml() const;

  std::vector<node> nodes_;
  std::vector<way> ways_;
  std::vector<relation> relations_;
};

struct id_partition {
  explicit id_partition(infrastructure const& infra)
      : first_element_{first_node_ + infra->graph_.nodes_.size()},
        first_station_{first_element_ + infra->graph_.elements_.size()},
        first_way_{as_val(first_station_) + infra->stations_.size()} {}

  osm::node::id get_id(station::ptr const s) const {
    return first_station_ + as_val(s->id_);
  }

  osm::node::id get_id(element::ptr const e) const {
    return first_element_ + as_val(e->get_id());
  }

  osm::node::id get_id(node::ptr const n) const {
    return first_node_ + as_val(n->get_id());
  }

  osm::node::id first_node_{0};
  osm::node::id first_element_{osm::node::id::invalid()};
  osm::node::id first_station_{osm::node::id::invalid()};

  osm::way::id first_way_{osm::way::id::invalid()};
};

osm::node get_osm_node(station::ptr const s, positions const& positions,
                       id_partition const& partition) {
  osm::node result;
  result.tags_.reserve(2);

  result.id_ = partition.get_id(s);
  result.position_ = positions.stations_[s->id_];
  result.tags_.emplace_back("type", "station");
  result.tags_.emplace_back("ds100", s->ds100_);

  return result;
}

osm::node get_osm_node(element::ptr const e, positions const& positions,
                       id_partition const& partition) {
  osm::node result;

  result.id_ = partition.get_id(e);
  result.position_ = positions.elements_[e->get_id()];

  result.tags_.reserve(4);
  result.tags_.emplace_back("type", "element");
  result.tags_.emplace_back("subtype", e->get_type_str());
  result.tags_.emplace_back("id", std::to_string(as_val(e->get_id())));

  if (e->is_track_element()) {
    result.tags_.emplace_back(
        std::pair{"direction", to_string(e->as<track_element>().dir())});
  }

  return result;
}

osm::node get_osm_node(node::ptr const n, positions const& positions,
                       id_partition const& partition) {
  osm::node result;

  result.id_ = partition.get_id(n);

  result.position_ = positions.nodes_[n->get_id()];

  result.tags_.reserve(4);
  result.tags_.emplace_back("type", "node");
  result.tags_.emplace_back("subtype", n->get_type_str());
  result.tags_.emplace_back("id", std::to_string(as_val(n->get_id())));

  return result;
}

osm::way get_osm_way(element::ptr const from, element::ptr const to,
                     id_partition const& partition) {
  osm::way result;

  result.nodes_.emplace_back(partition.get_id(from));
  result.nodes_.emplace_back(partition.get_id(to));

  result.tags_.emplace_back("railway", "rail");
  result.tags_.emplace_back("type", "element");

  return result;
}

osm::way get_osm_way(node::ptr const from, node::ptr const to,
                     id_partition const& partition) {
  osm::way result;

  result.nodes_.emplace_back(partition.get_id(from));
  result.nodes_.emplace_back(partition.get_id(to));

  result.tags_.emplace_back("railway", "rail");
  result.tags_.emplace_back("type", "node");

  return result;
}

pugi::xml_document osm::to_xml() const {
  utl::scoped_timer const timer("exporting osm to xml");

  auto const append_tags = [](pugi::xml_node xml_node, osm::tags const& tags) {
    for (auto const& tag : tags) {
      auto tag_xml = xml_node.append_child("tag");
      tag_xml.append_attribute("k").set_value(tag.first.c_str());
      tag_xml.append_attribute("v").set_value(tag.second.c_str());
    }
  };

  pugi::xml_document result;

  pugi::xml_node osm_node = result.append_child("osm");
  auto version = osm_node.append_attribute("version");
  version.set_value("0.6");

  for (auto const& node : nodes_) {
    auto xml_node = osm_node.append_child("node");

    xml_node.append_attribute("id").set_value(as_val(node.id_));

    xml_node.append_attribute("lon").set_value(node.position_.lon_);
    xml_node.append_attribute("lat").set_value(node.position_.lat_);

    append_tags(xml_node, node.tags_);
  }

  for (auto const& way : ways_) {
    auto xml_way = osm_node.append_child("way");

    xml_way.append_attribute("id").set_value(as_val(way.id_));

    for (auto const& node : way.nodes_) {
      xml_way.append_child("nd").append_attribute("ref").set_value(
          as_val(node));
    }

    append_tags(xml_way, way.tags_);
  }

  return result;
}

osm get_osm(infrastructure const& infra, positions const& positions) {
  utl::scoped_timer const timer("creating osm data");

  osm result;

  auto const partition = id_partition(infra);

  for (auto const& node : infra->graph_.nodes_) {
    result.nodes_.emplace_back(get_osm_node(node, positions, partition));
  }

  for (auto const& element : infra->graph_.elements_) {
    result.nodes_.emplace_back(get_osm_node(element, positions, partition));
  }

  for (auto const& station : infra->stations_) {
    result.nodes_.emplace_back(get_osm_node(station, positions, partition));
  }

  for (auto const& n : infra->graph_.nodes_) {
    if (n->next_ != nullptr) {
      result.ways_.emplace_back(get_osm_way(n, n->next_, partition));
    }

    if (n->branch_ != nullptr) {
      result.ways_.emplace_back(get_osm_way(n, n->branch_, partition));
    }
  }

  for (auto const& e : infra->graph_.elements_) {
    for (auto const& neighbour : e->neighbours()) {
      // hacky, but cool
      // only create ways to neighbours with a higher memory address
      if (neighbour > e) continue;

      result.ways_.emplace_back(get_osm_way(e, neighbour, partition));
    }
  }

  // set way ids, as we did not determine the total amount of ways beforehand
  for (auto [id, way] : utl::enumerate<osm::way::id>(result.ways_)) {
    way.id_ = partition.first_way_ + id;
  }

  utls::ensure(result.ok(), "result not ok");

  return result;
}

void export_and_write2(infrastructure const& infra, positions const& positions,
                       std::filesystem::path const& out) {
  utl::scoped_timer const timer("create and export osm");

  auto const osm = get_osm(infra, positions);
  auto const osm_xml = osm.to_xml();

  osm_xml.save_file(out.c_str());
}

}  // namespace soro::server
