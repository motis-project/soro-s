#include "soro/server/modules/timetable/timetable_module.h"

#include <filesystem>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "utl/logging.h"

#include "soro/timetable/timetable.h"
#include "soro/timetable/timetable_options.h"

#include "soro/server/server_settings.h"

#include "soro/server/modules/infrastructure/infrastructure_module.h"

namespace soro::server {

namespace fs = std::filesystem;

tt::timetable::optional_ptr timetable_module::get_timetable(
    std::string const& infrastructure_name,
    std::string const& timetable_name) const {

  auto const context_it = contexts_.find(infrastructure_name);

  if (context_it == std::end(contexts_)) {
    return std::nullopt;
  }

  auto const& context = context_it->second;

  auto tt_it = context.timetables_.find(timetable_name);
  if (tt_it == std::end(context.timetables_)) {
    return std::nullopt;
  }

  return tt::timetable::optional_ptr(tt_it->second.get());
}

std::vector<fs::path> get_timetable_todo_list(server_settings const& settings) {
  std::vector<fs::path> timetable_todo_list;

  for (auto&& dir_entry :
       fs::directory_iterator{settings.timetable_sources_.val()}) {

    if (!dir_entry.is_directory()) {
      uLOG(utl::info) << "ignoring non-directory entry " << dir_entry
                      << " in timetable resource directory";
      continue;
    }

    auto const potential_raw =
        settings.server_timetable_dir() / dir_entry.path().filename();

    // add to todo list if either:
    // - regenerate is set
    // - compiled without serialization support
    // - we don't have a serialized raw file anyways
    // - the serialized raw file we have is outdated
    if (settings.regenerate_.val() ||
        !tt::timetable::serialization_possible() || !exists(potential_raw) ||
        fs::last_write_time(potential_raw) < dir_entry.last_write_time()) {
      timetable_todo_list.emplace_back(dir_entry.path());
    } else {
      timetable_todo_list.emplace_back(potential_raw);
    }
  }

  return timetable_todo_list;
}

timetable_module get_timetable_module(server_settings const& s,
                                      infrastructure_module const& infra_m) {
  timetable_module result;

  auto const timetable_todo_list = get_timetable_todo_list(s);

  for (auto const& infra_context : infra_m.all()) {
    timetable_module::infra_context context;

    for (auto const& tt_item : timetable_todo_list) {
      auto tt = tt::try_parsing_timetable(tt::make_timetable_opts(tt_item),
                                          infra_context.infra_);

      if (!tt) {
        continue;
      }

      auto tt_ptr = std::make_unique<tt::timetable>(std::move(*tt));

      if (tt::timetable::serialization_possible() && is_directory(tt_item)) {
        (*tt_ptr).save(s.server_timetable_dir() / tt_item.filename());
      }

      context.timetables_.emplace((*tt_ptr)->source_, std::move(tt_ptr));
    }

    result.contexts_.emplace(infra_context.infra_->source_, std::move(context));
  }

  return result;
}

}  // namespace soro::server
