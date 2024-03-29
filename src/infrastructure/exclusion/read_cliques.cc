#include "soro/infrastructure/exclusion/read_cliques.h"

#include <fstream>

#include "utl/timer.h"

#include "soro/utls/parse_int.h"
#include "soro/utls/std_wrapper/sort.h"
#include "soro/utls/string.h"

namespace soro::infra {

soro::vector<interlocking_route::ids> read_cliques(
    std::filesystem::path const& clique_fp) {
  utl::scoped_timer const timer("reading cliques");

  soro::vector<interlocking_route::ids> cliques;

  std::ifstream in(clique_fp);

  for (std::string line; getline(in, line);) {
    auto const split = utls::split(line, " ");

    cliques.emplace_back(soro::to_vec(split, [](auto&& s) {
      return utls::parse_int<interlocking_route::id>(s);
    }));
  }

  for (auto& clique : cliques) {
    utls::sort(clique);
  }

  utls::sort(cliques);

  return cliques;
}

}  // namespace soro::infra
