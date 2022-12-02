#include "soro/timetable/base_timetable.h"

#include "utl/pipes.h"

namespace soro::tt {

auto get_filtered_train_runs(soro::vector<soro::unique_ptr<train>>&,
                             utls::unixtime const, utls::unixtime const) {
  //  train::id current_id = 0;

  //  return utl::all(trains) | utl::remove_if([&](auto&& tr_ptr) {
  //           return !tr_ptr->has_event_in_interval(start, end);
  //         }) |
  //         utl::transform([&](auto&& tr_ptr) -> train::ptr {
  //           tr_ptr->id_ = current_id++;
  //           return tr_ptr.get();
  //         }) |
  //         utl::emplace_back<soro::vector<train::ptr>>();
}

// auto get_name_to_train(soro::vector<train::ptr> const& trs) {
//   return utl::all(trs) |
//          utl::transform([](auto&& tr) -> soro::pair<soro::string, train::ptr>
//          {
//            return {tr->name_, tr};
//          }) |
//          utl::insert<soro::map<soro::string, train::ptr>>();
// }

base_timetable make_base_timetable(soro::vector<soro::unique_ptr<train>> const&,
                                   timetable_options const& opts) {
  base_timetable bt;

  bt.start_ = opts.start_;
  bt.end_ = opts.end_;

  //  bt.train_store_ = std::move(trains);
  //  bt.trains_ = get_filtered_train_runs(bt.train_store_, opts.start_,
  //  opts.end_); bt.name_to_train_ = get_name_to_train(bt.trains_);

  // No name conflicts allowed
  //  assert(bt.name_to_train_.size() == bt.trains_.size());

  return bt;
}

}  // namespace soro::tt
