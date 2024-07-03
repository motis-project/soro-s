#pragma once

#include "soro/infrastructure/graph/graph.h"

namespace soro::infra {

bool joins_lines(end_element const&);
bool joins_lines(track_element const&);
bool joins_lines(simple_element const&);
bool joins_lines(simple_switch const& s);
bool joins_lines(cross const& c);

bool misaligned_join(end_element const&);
bool misaligned_join(track_element const&);
bool misaligned_join(simple_element const&);
bool misaligned_join(simple_switch const& s, graph const& g);
bool misaligned_join(cross const& c, graph const& g);

}  // namespace soro::infra