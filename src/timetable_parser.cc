#include "soro/timetable_parser.h"

#include <sstream>

#include "date/date.h"

#include "utl/parser/buf_reader.h"
#include "utl/parser/csv_range.h"
#include "utl/parser/line_range.h"
#include "utl/pipes.h"

#include "soro/network.h"
#include "soro/speed_t.h"

namespace soro {

unixtime parse_time(utl::cstr source) {
  date::sys_seconds t;
  std::istringstream{source.to_str()} >> date::parse("%F %T", t);
  return unixtime{
      static_cast<time_t>(std::chrono::duration_cast<std::chrono::seconds>(
                              t - date::sys_seconds(std::chrono::seconds{0}))
                              .count())};
}

timetable parse_timetable(network const& net, std::string_view trains_csv,
                          std::string_view timetable_csv) {
  struct train_row {
    utl::csv_col<utl::cstr, UTL_NAME("TRAIN")> name_;
    utl::csv_col<speed_t, UTL_NAME("SPEED")> speed_;
  };
  auto tt = utl::line_range{utl::buf_reader{trains_csv}}  //
            | utl::csv<train_row>()  //
            |
            utl::transform([](auto&& row) {
              return cista::pair{row.name_.val().to_str(),
                                 cista::raw::make_unique<train>(
                                     row.name_.val().to_str(), row.speed_.val(),
                                     std::vector<train::timetable_entry>{},
                                     std::vector<std::unique_ptr<route>>{})};
            })  //
            | utl::to<timetable>();

  struct timetable_row {
    utl::csv_col<utl::cstr, UTL_NAME("TRAIN")> name_;
    utl::csv_col<utl::cstr, UTL_NAME("POSITION")> pos_;
    utl::csv_col<utl::cstr, UTL_NAME("TIME")> time_;
  };
  utl::line_range{utl::buf_reader{timetable_csv}}  //
      | utl::csv<timetable_row>()  //
      | utl::for_each([&](auto&& row) {
          auto const train_it = tt.find(row.name_.val().view());
          utl::verify(train_it != end(tt), "train {} not found",
                      row.name_.val().view());

          auto const pos_it = std::find_if(
              begin(net.nodes_), end(net.nodes_),
              [&](auto&& n) { return n->name_ == row.pos_.val().view(); });
          utl::verify(pos_it != end(net.nodes_), "position {} not found",
                      row.pos_.val().view());

          train_it->second->timetable_.emplace_back(train::timetable_entry{
              parse_time(row.time_.val()), pos_it->get()});
        });

  for (auto& [name, t] : tt) {
    t->build_routes(net);
  }

  return tt;
}

}  // namespace soro