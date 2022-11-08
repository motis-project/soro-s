#include "soro/timetable/parsers/csv/parse_csv_timetable.h"

#include "tar/tar_reader.h"

#include "utl/logging.h"
#include "utl/parser/buf_reader.h"
#include "utl/parser/csv_range.h"
#include "utl/parser/line_range.h"
#include "utl/pipes.h"

#include "soro/utls/parse_fp.h"
#include "soro/utls/std_wrapper/std_wrapper.h"

#include "soro/timetable/parsers/raw_to_trains.h"

namespace soro::tt {

using namespace soro::utls;
using namespace soro::infra;

namespace fs = std::filesystem;

std::vector<fs::path> const required_files = {"trains.txt", "train_physics.txt",
                                              "train_runs.txt"};

const char* const time_format_string = "%FT%T%Ez";

utls::duration const global_minimum_stop_time{120};

bool is_csv_timetable(std::vector<utls::loaded_file> const& archive_files) {
  return archive_files.size() == required_files.size() &&
         utls::all_of(required_files, [&](auto&& required) {
           return utls::any_of(archive_files, [&](auto&& loaded_file) {
             return loaded_file.path_.filename() == required;
           });
         });
}

bool is_csv_timetable(timetable_options const& opts) {
  return fs::is_directory(opts.timetable_path_) &&
         utls::all_of(required_files, [&](auto&& filename) {
           return fs::exists(opts.timetable_path_ / filename);
         });
}

std::vector<raw_train::run> get_raw_train_runs(
    utls::loaded_file const& runs_file, infrastructure const& infra) {
  std::vector<raw_train::run> runs;

  struct run_row {
    utl::csv_col<std::size_t, UTL_NAME("id")> id_;
    utl::csv_col<std::string, UTL_NAME("station")> ds100_;
    utl::csv_col<std::string, UTL_NAME("route")> route_;
    utl::csv_col<utl::cstr, UTL_NAME("arrival")> arrival_;
    utl::csv_col<utl::cstr, UTL_NAME("departure")> departure_;
  };

  utl::line_range{utl::buf_reader{runs_file.contents_}} | utl::csv<run_row>() |
      utl::for_each(
          [&](auto&& row) {
            if (runs.size() < row.id_ + 1) {
              runs.resize(row.id_ + 1);
            }

            auto& run = runs[row.id_.val()];

            auto station_it = infra->ds100_to_station_.find(row.ds100_.val());
            if (station_it == std::end(infra->ds100_to_station_)) {
              runs[row.id_] = {};
              uLOG(utl::warn)
                  << "Could not find station " << row.ds100_.val().data()
                  << " in infrastructure while parsing timetable entries.";
              return;
            }
            auto const& station = station_it->second;

            auto sr_it =
                station->station_routes_.find(soro::string(row.route_.val()));
            if (sr_it == std::cend(station->station_routes_)) {
              uLOG(utl::warn)
                  << "Could not find station route " << row.route_.val()
                  << " in station " << station->ds100_
                  << " while parsing timetable entries.";
              runs[row.id_] = {};
              return;
            }
            auto route = sr_it->second;
            run.routes_.push_back(route);

            utls::unixtime const arrival =
                row.arrival_.val().empty()
                    ? utls::unixtime{}
                    : utls::unixtime{row.arrival_.val().c_str(),
                                     time_format_string};
            run.arrivals_.push_back(arrival);

            utls::unixtime const departure =
                row.departure_.val().empty()
                    ? utls::unixtime{}
                    : utls::unixtime{row.departure_.val().c_str(),
                                     time_format_string};
            run.departures_.push_back(departure);

            utls::duration const min_stop_time = std::max(
                global_minimum_stop_time, (departure - arrival).as_duration());
            run.min_stop_times_.push_back(min_stop_time);
          });

  return runs;
}

std::vector<raw_train::physics> get_raw_train_physics(
    utls::loaded_file const& types_file) {

  std::vector<raw_train::physics> raw_train_physics;

  struct type_row {
    utl::csv_col<std::size_t, UTL_NAME("id")> id_;
    utl::csv_col<utl::cstr, UTL_NAME("series")> series_;
    utl::csv_col<utl::cstr, UTL_NAME("owner")> owner_;
    utl::csv_col<rs::variant_id, UTL_NAME("variant")> variant_;
    utl::csv_col<std::string, UTL_NAME("max_speed")> max_speed_;
    utl::csv_col<std::string, UTL_NAME("weight")> weight_;
    utl::csv_col<std::string, UTL_NAME("length")> length_;
  };

  utl::line_range{utl::buf_reader{types_file.contents_}} |
      utl::csv<type_row>() | utl::for_each([&](auto&& row) {
        raw_train::physics tp;
        tp.series_ = soro::string{row.series_.val().to_str()};
        tp.owner_ = soro::string{row.owner_.val().to_str()};
        tp.variant_ = row.variant_.val();
        tp.carriage_weight_ = si::from_ton(
            utls::parse_fp<si::precision>(row.weight_.val().data()));
        tp.max_speed_ = si::from_km_h(
            utls::parse_fp<si::precision>(row.max_speed_.val().data()));
        tp.length_ =
            si::from_m(utls::parse_fp<si::precision>(row.length_.val().data()));

        raw_train_physics.emplace_back(tp);
      });

  return raw_train_physics;
}

std::vector<raw_train> get_raw_trains(utls::loaded_file const& train_file) {
  std::vector<raw_train> raw_trains;

  struct train_row {
    utl::csv_col<std::size_t, UTL_NAME("id")> id_;
    utl::csv_col<utl::cstr, UTL_NAME("name")> name_;
    utl::csv_col<bool, UTL_NAME("freight")> freight_;
    utl::csv_col<bool, UTL_NAME("ctc")> ctc_;
  };

  utl::line_range{utl::buf_reader{train_file.contents_}} |
      utl::csv<train_row>() | utl::for_each([&](auto&& row) {
        raw_train rt;
        rt.id_ = row.id_.val();
        rt.name_ = soro::string{row.name_.val().to_str()};
        rt.freight_ = static_cast<rs::FreightTrain>(row.freight_.val());
        rt.ctc_ = static_cast<rs::CTC>(row.ctc_.val());
        raw_trains.emplace_back(rt);
      });

  return raw_trains;
}

std::vector<raw_train> parse_csv_timetable(
    std::map<std::string, utls::loaded_file> const& loaded_csv_tt,
    infrastructure const& infra) {

  auto raw_trains = get_raw_trains(loaded_csv_tt.at("trains.txt"));
  auto const raw_train_runs =
      get_raw_train_runs(loaded_csv_tt.at("train_runs.txt"), infra);
  auto const raw_train_physics =
      get_raw_train_physics(loaded_csv_tt.at("train_physics.txt"));

  for (auto& raw_train : raw_trains) {
    utl::verify(raw_train_runs.size() > raw_train.id_,
                "Found train with id {}, but no associated train run",
                raw_train.id_);
    utl::verify(!raw_train_runs[raw_train.id_].routes_.empty(),
                "Found train with id {}, but empty associated train run",
                raw_train.id_);

    raw_train.run_ = raw_train_runs[raw_train.id_];
    raw_train.physics_ = raw_train_physics[raw_train.id_];
  }

  return raw_trains;
}

base_timetable parse_csv_timetable(timetable_options const& opts,
                                   infrastructure const& infra) {
  std::map<std::string, utls::loaded_file> loaded_csv_timetable;
  for (auto const& required : required_files) {
    auto const path = opts.timetable_path_ / required;
    loaded_csv_timetable[required.string()] = utls::loaded_file{
        .path_ = path, .contents_ = utls::read_file_to_string(path)};
  }

  auto const raw_trains = parse_csv_timetable(loaded_csv_timetable, infra);
  auto uniques =
      raw_trains_to_trains(raw_trains, infra->interlocking_,
                           infra->station_route_graph_, infra->rolling_stock_);

  return make_base_timetable(std::move(uniques), opts);
}

}  // namespace soro::tt
