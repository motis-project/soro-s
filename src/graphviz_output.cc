#include "soro/graphviz_output.h"

#include <ostream>
#include <sstream>

#include "cista/containers/hash_map.h"

#include "utl/enumerate.h"

#include "soro/network.h"
#include "soro/train.h"

namespace cr = cista::raw;

namespace soro {

void graphiz_output(std::ostream& out, timetable const& tt) {
  out << "digraph world {\n";
  out << R"--(
    rankdir="LR";

    node [
      fixedsize=false,
      fontname=Monospace,
      fontsize=11,
      height=2,
      width=3.2,
      style=bold,
      color=white,
      fillcolor=white,
      shape=box
    ];

    edge [
      color=white
    ];

)--";
  for (auto const& [name, t] : tt) {
    out << "    subgraph cluster_" << t->name_ << " {\n";
    out << "        style=filled;\n";
    out << "        color=lightgrey;\n";
    out << "        label=\"Train " << t->name_ << "\"\n";
    out << "        rank=\"same\"\n";

    cr::hash_map<route*, std::vector<route*>> main;
    for (auto const& r : t->routes_) {
      main[r->main_].emplace_back(r.get());
    }

    for (auto const& [m, routes] : main) {
      if (m == nullptr || routes.size() == 1) {
        for (auto const& r : routes) {
          if (r->from_ != nullptr) {
            out << "        " << r->tag() << "\n";
          }
        }
      } else {
        out << "        subgraph cluster_main_" << m->tag() << " {\n";
        out << "            style=bold;\n";
        out << "            color=dodgerblue;\n";
        out << "            label=\"Route " << name << ": "
            << routes.front()->from_->name_ << " -> "
            << routes.back()->to_->name_ << "\"\n";
        out << "            rank=\"same\"\n";
        for (auto const& r : routes) {
          out << "            " << r->tag() << "\n";
        }
        out << "        }\n";
      }
    }
    out << "    }\n";
  }

  auto const is_delayed_by_other = [&](route const* r) {
    auto maybe = false;
    for (auto const& in : r->in_) {
      maybe |= in->train_ != r->train_ &&
               r->entry_dpd_.first_ == in->eotd_dpd_.first_;
      if (in->train_ == r->train_ &&
          r->entry_dpd_.first_ == in->exit_dpd_.first_) {
        return false;
      }
    }
    return maybe;
  };
  auto const printable_delay = [](unixtime const diff) {
    auto const total_seconds = static_cast<time_t>(diff);
    auto const sec = total_seconds % 60;
    auto const min = total_seconds / 60;
    std::stringstream ss;
    if (total_seconds > 0) {
      ss << "+";
    }
    if (min != 0) {
      ss << min << "min";
    }
    if (sec != 0) {
      ss << (min == 0 ? "" : " ") << sec << "s";
    }
    return ss.str();
  };
  for (auto const& [name, t] : tt) {
    for (auto const& r : t->routes_) {
      out << "    " << r->tag() << R"([URL="#)" << r->tag() << "\""
          << (r->from_ == nullptr ? R"(, color=darkviolet, shape=ellipse)"
              : is_delayed_by_other(r.get()) ? ", color=red, fontcolor=red"
                                             : ", color=green3")
          << R"(, label=")" << r->train_->name_ << ": "
          << (r->from_ == nullptr ? "START" : r->from_->name_) << " -> "
          << r->to_->name_ << "\\n"
          << "sched@" << (r->from_ == nullptr ? "START" : r->from_->name_)
          << " = " << r->from_time_ << "\\n"
          << "sched@" << r->to_->name_ << " = " << r->to_time_ << "\\n";
      for (auto const& [t, speed_dpb] : r->entry_dpd_) {
        for (auto const [speed, prob] : speed_dpb) {
          out << t
              << (t - r->from_time_ == 0
                      ? ""
                      : " (" + printable_delay(t - r->from_time_) + ")")
              << " @ " << speed << "km/h"
              << ": " << (prob * 100) << "%\\n";
        }
      }
      out << "\" ];\n";
    }
  }
  out << "\n";
  for (auto const& [name, t] : tt) {
    for (auto const& r : t->routes_) {
      for (auto const& o : r->out_) {
        out << "    " << r->tag() << " -> " << o->tag();
        if (r->from_ == nullptr) {
          out << " [fontcolor=darkviolet, color=darkviolet, style=\"bold\", "
                 "label=\""
              << r->to_time_ << "\"]";
        } else if (r->train_ != o->train_) {
          out << R"( [fontcolor=red, color=red, style="bold", label=")"
              << r->eotd_dpd_.first_ << "\"]";
        } else {
          out << " [fontcolor=white, label=\"" << r->exit_dpd_.first_ << "\"]";
        }
        out << ";\n";
      }
    }
  }
  out << "}\n";
}

}  // namespace soro