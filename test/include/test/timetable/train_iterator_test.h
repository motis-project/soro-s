#pragma once

#include "soro/infrastructure/infrastructure.h"
#include "soro/timetable/train.h"

namespace soro::tt::test {

void do_train_iterator_tests(train const&, infra::infrastructure const&);

}  // namespace soro::tt::test
