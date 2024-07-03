#pragma once

namespace soro::infra {

struct construction_work {
  interval interval_;
  soro::vector<station::id> involved_stations_;
  soro::string id_;
};

std::vector<construction_work> parse_construction_work(
    infrastructure const& infra, infrastructure_options const& opts) {
  utl::scoped_timer const scoped_timer("parsing construction work");

  std::vector<construction_work> result;

  iss_files iss_files(opts.infrastructure_path_);

  for (auto const& construction_file : iss_files.construction_work_files_) {

    for (auto const construction_work_set_xml :
         construction_file.child(XML_ISS_DATA)
             .child("BaubetrieblicheMassnahmen")
             .children()) {

      for (auto const construction_work_xml :
           construction_work_set_xml.child("BetrieblicheVEn")
               .children("BetrieblicheVE")) {

        for (auto const rules_xml :
             construction_work_xml.child("BaubetrieblicheRegelungen")
                 .children("BaubetrieblicheRegelung")) {
          construction_work cw;

          cw.id_ = rules_xml.child_value("BBRID");

          auto const valid_xml = rules_xml.child("Gueltigkeit-BBR");

          cw.interval_.start_ = parse_dmy_hms(valid_xml.child_value("Begin"));
          cw.interval_.end_ = parse_dmy_hms(valid_xml.child_value("Ende"));

          for (auto const& station_xml :
               rules_xml.child("MakroskopischeOrtsangabe")
                   .child("Betriebsstellen")
                   .children("Betriebsstelle")) {

            auto const station_it =
                infra->ds100_to_station_.find(station_xml.child_value());
            if (station_it != std::end(infra->ds100_to_station_)) {
              cw.involved_stations_.emplace_back(station_it->second->id_);
            } else {
              std::cout << "station not found: " << station_xml.child_value()
                        << "\n";
            }
          }

          result.emplace_back(cw);
        }
      }
    }
  }

  return result;
}

void get_construction_for_train(
    infrastructure const& infra, timetable const& tt,
    std::vector<construction_work> const& construction,
    train::id const train_id, interval const& inter) {

  std::vector<construction_work> time_filtered;

  for (auto const& cw : construction) {
    if (cw.interval_.overlaps(inter)) {
      time_filtered.emplace_back(cw);
    }
  }

  std::vector<station::id> train_station_ids;
  for (auto const& sp : tt->trains_[train_id].sequence_points_) {
    auto const& sr = infra->station_routes_[sp.station_route_];
    train_station_ids.emplace_back(sr->station_->id_);
  }

  std::vector<construction_work> station_filtered_without_time;
  for (auto const& cw : construction) {
    if (utls::any_of(cw.involved_stations_, [&](auto&& id) {
          return utls::contains(train_station_ids, id);
        })) {
      station_filtered_without_time.emplace_back(cw);
    }
  }

  std::vector<construction_work> station_filtered;
  for (auto const& cw : time_filtered) {
    if (utls::any_of(cw.involved_stations_, [&](auto&& id) {
          return utls::contains(train_station_ids, id);
        })) {
      station_filtered.emplace_back(cw);
    }
  }

  std::ignore = station_filtered;
}

}  // namespace soro::infra
