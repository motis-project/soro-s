#include "test/ordering/all_train_edges_exist.h"

#include "soro/utls/print_progress.h"
#include "soro/utls/std_wrapper/all_of.h"
#include "soro/utls/std_wrapper/contains.h"

#include "soro/ordering/graph.h"

namespace soro::ordering::test {

using namespace soro::tt;
using namespace soro::infra;

bool all_train_edges_exist(graph const& og) {
  auto const neither_next_or_prev_is_missing = [&og](auto&& node) {
    utls::print_progress("checking if all train edges exist", og.nodes_);
    auto const next_is_missing =
        node.has_next(og) && !utls::contains(node.out(og), node.next_id(og));
    auto const prev_is_missing =
        node.has_prev(og) && !utls::contains(node.in(og), node.prev_id(og));

    return !next_is_missing && !prev_is_missing;
  };

  return utls::all_of(og.nodes_, neither_next_or_prev_is_missing);
}

}  // namespace soro::ordering::test