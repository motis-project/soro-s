#include "soro/infrastructure/exclusion/read_cliques.h"

#include <fstream>

#include "soro/utls/parse_int.h"
#include "soro/utls/std_wrapper/std_wrapper.h"
#include "soro/utls/string.h"

namespace soro::infra {

soro::vector<exclusion_set> read_cliques(
    std::filesystem::path const& clique_fp) {
  soro::vector<soro::vector<interlocking_route::id>> cliques;

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

  return soro::to_vec(cliques, [](auto&& c) { return make_exclusion_set(c); });
}

}  // namespace soro::infra
