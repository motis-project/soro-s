#pragma once

#include "soro/ordering/graph.h"

namespace soro::ordering::test {

bool check_train_edge_transitivity(graph const& og,
                                   infra::infrastructure const& infra,
                                   tt::timetable const& tt);

}  // namespace soro::ordering::test
