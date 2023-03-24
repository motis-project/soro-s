#include "soro/server/modules/infrastructure/infrastructure_module.h"

#include "utl/logging.h"
#include "utl/timer.h"

#include "soro/utls/serializable.h"

using namespace soro::infra;

namespace fs = std::filesystem;

namespace soro::server {

infra::infrastructure::optional_ptr infrastructure_module::get_infra(
    std::string_view const name) const {

  if (auto const infra_it = infrastructures_.find(name);
      infra_it != std::end(infrastructures_)) {
    return infrastructure::optional_ptr(infra_it->second.get());
  }

  return {};
}

std::vector<fs::path> get_infrastructure_todo_list(
    soro::server::server_settings const& settings) {

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
  using namespace soro::infra;

  utl::scoped_timer const timer("creating infrastructure state");

  infrastructure_module result;

  auto const infra_todo_list = get_infrastructure_todo_list(s);
  for (auto const& infra_item : infra_todo_list) {
    auto infra = std::make_unique<infrastructure>(
        is_directory(infra_item)
            ? infrastructure(make_infra_opts(infra_item, s.coord_file()))
            : infrastructure(infra_item));

    if (infrastructure::serialization_possible() && is_directory(infra_item)) {
      (*infra).save(s.server_infra_dir() / infra_item.filename());
    }

    result.infrastructures_.emplace((*infra)->source_, std::move(infra));
  }

  return result;
}

}  // namespace soro::server
