#include "soro/server/modules/infrastructure/infrastructure_module.h"

#include <filesystem>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "utl/logging.h"
#include "utl/timer.h"

#include "soro/base/soro_types.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/infrastructure_options.h"

#include "soro/server/server_settings.h"

#include "soro/server/modules/infrastructure/parse_station_positions.h"
#include "soro/server/modules/infrastructure/positions.h"

namespace soro::server {

namespace fs = std::filesystem;

using namespace soro::infra;

soro::optional<infrastructure_module::context::ptr>
infrastructure_module::get_context(std::string const& context_name) const {

  if (auto const it = contexts_.find(context_name); it != std::end(contexts_)) {
    return soro::optional<infrastructure_module::context::ptr>{&(it->second)};
  }

  return {};
}

std::vector<fs::path> get_infrastructure_todo_list(
    server_settings const& settings) {
  std::vector<fs::path> infra_todo_list;

  for (auto&& dir_entry :
       fs::directory_iterator{settings.infrastructure_sources_.val()}) {
    if (!dir_entry.is_directory()) {
      uLOG(utl::info) << "ignoring non-directory entry " << dir_entry
                      << " in infrastructure resource directory";
      continue;
    }

    auto const potential_raw =
        settings.server_infra_dir() / dir_entry.path().filename();

    // add to todo list if either:
    // - regenerate is set
    // - compiled without serialization support
    // - we don't have a serialized raw file anyways
    // - the serialized raw file we have is outdated
    if (settings.regenerate_.val() ||
        !soro::infra::infrastructure::serialization_possible() ||
        !exists(potential_raw) ||
        fs::last_write_time(potential_raw) < dir_entry.last_write_time()) {
      infra_todo_list.emplace_back(dir_entry.path());
    } else {
      infra_todo_list.emplace_back(potential_raw);
    }
  }

  return infra_todo_list;
}

infrastructure_module get_infrastructure_module(
    soro::server::server_settings const& s) {
  utl::scoped_timer const timer("creating infrastructure module");

  infrastructure_module result;

  result.station_positions_ = parse_station_positions(s.coord_file());

  auto const infra_todo_list = get_infrastructure_todo_list(s);
  for (auto const& infra_item : infra_todo_list) {
    infrastructure_module::context context;

    context.infra_ =
        is_directory(infra_item)
            ? infrastructure(make_infra_opts(infra_item, s.coord_file()))
            : infrastructure(infra_item);

    if (infrastructure::serialization_possible() && is_directory(infra_item)) {
      context.infra_.save(s.server_infra_dir() / infra_item.filename());
    }

    context.positions_ =
        get_positions(context.infra_, result.station_positions_);

    result.contexts_.emplace(context.infra_->source_, std::move(context));
  }

  return result;
}

}  // namespace soro::server
