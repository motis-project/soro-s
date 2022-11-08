#include "emscripten/bind.h"

#include "soro/timetable/timetable.h"

#include "soro/web/register_vector.h"

using namespace soro;
using namespace soro::tt;
using namespace soro::infra;

EMSCRIPTEN_BINDINGS(timetable_options) {
  emscripten::class_<timetable_options>("TimetableOptions")
      .constructor<>()
      .property("timetable_path", &timetable_options::timetable_path_);
}

auto const& get_dispo_train_runs(timetable const& tt) {
  return tt->trains_;
}

EMSCRIPTEN_BINDINGS(timetable) {
  register_vector<train::ptr>("TrainList");

  emscripten::class_<train>("Train")
      .property("name", &train::name_);

  emscripten::class_<struct timetable>("Timetable")
      .constructor<timetable_options const&,
                   infrastructure const&>()
      .constructor<std::filesystem::path>()
      .property("trainRuns", &get_dispo_train_runs)
      .function("save", &timetable::save);
}
