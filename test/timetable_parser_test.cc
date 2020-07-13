#include "doctest/doctest.h"

#include <iostream>

#include "utl/pairwise.h"
#include "utl/verify.h"

#include "rapid/ascii_network_parser.h"
#include "rapid/timetable_parser.h"

using namespace rapid;

struct r {
  cista::raw::hash_set<route*> in_;
};

TEST_CASE("timetable_parser") {
  auto const net =
      parse_network("a===)=A>=]======)=B>=]======)=C>=]======)=D>=]===b");
  auto const tt = parse_timetable(net, R"(TRAIN,SPEED
X,50
Y,100
Z,200
)",
                                  R"(TRAIN,POSITION,TIME
X,a,2020-01-01 13:00:00
X,b,2020-01-01 14:09:00
Y,a,2020-01-01 13:32:00
Y,b,2020-01-01 14:07:00
Z,a,2020-01-01 13:49:00
Z,b,2020-01-01 14:07:00
)");

  namespace cr = cista::raw;
  cr::hash_map<cr::pair<node*, node*>, std::vector<route*>> route_train_order;
  for (auto& [name, t] : tt) {
    route* pred{nullptr};
    for (auto const& r : t->routes_) {
      if (pred != nullptr) {
        r->in_.emplace(pred);
        pred->out_.emplace(r.get());
      }
      if (r->from_ != nullptr) {
        auto& train_order = route_train_order[{r->from_, r->to_}];
        train_order.emplace(
            std::lower_bound(begin(train_order), end(train_order), r.get(),
                             [](route const* a, route const* b) {
                               return a->from_time_ < b->from_time_;
                             }),
            r.get());
      }
      pred = r.get();
    }
  }

  for (auto const& [fromto, routes] : route_train_order) {
    std::cout << (fromto.first == nullptr ? "" : fromto.first->name_) << " -> "
              << fromto.second->name_ << "\n";
    for (auto const& r : routes) {
      std::cout << "  " << *r << "\n";
    }
  }

  for (auto const& [fromto, routes] : route_train_order) {
    for (auto const& [r1, r2] : utl::pairwise(routes)) {
      if (r2->pred_->from_ != nullptr) {
        r1->out_.emplace(r2->pred_);
        r2->pred_->in_.emplace(r1);
      }
    }
  }

  cr::hash_set<route*> todo;
  for (auto const& [fromto, routes] : route_train_order) {
    for (auto const& r : routes) {
      if (r->finished()) {
        continue;
      } else {
        if (r->ready()) {
          todo.emplace(r);
        }
      }
    }
  }

  while (!todo.empty()) {
    auto const next = *begin(todo);
    todo.erase(begin(todo));
    next->compute_dists();
    for (auto const& out : next->out_) {
      if (out->ready()) {
        todo.emplace(out);
      }
    }
  }

  std::cout << "digraph world {\n";
  std::cout << R"--(
    node [
      fixedsize=false,
      fontname=Monospace,
      fontsize=12,
      height=2,
      width=3
    ];
)--";
  for (auto const& [name, t] : tt) {
    std::cout << "    {rank=same; ";
    for (auto const& r : t->routes_) {
      std::cout << r->tag() << " ";
    }
    std::cout << ";}\n";
  }
  for (auto const& [name, t] : tt) {
    for (auto const& r : t->routes_) {
      std::cout
          << "    " << r->tag()
          << R"( [ shape=box, fixedsize=false, style = "filled, bold", label=")"
          << r->train_->name_ << ": "
          << (r->from_ == nullptr ? "START" : r->from_->name_) << " -> "
          << r->to_->name_ << "\\n"
          << "sched=" << r->from_time_ << "\\n";
      for (auto const [t, speed_dpb] : r->entry_dpd_) {
        for (auto const [speed, prob] : speed_dpb) {
          std::cout << t << " @ " << speed << "km/h"
                    << ": " << (prob * 100) << "%\\n";
        }
      }
      std::cout << "\" ];\n";
    }
  }
  std::cout << "\n";
  for (auto const& [name, t] : tt) {
    for (auto const& r : t->routes_) {
      for (auto const& out : r->out_) {
        std::cout << "    " << r->tag() << " -> " << out->tag() << ";\n";
      }
    }
  }
  std::cout << "}\n";
}