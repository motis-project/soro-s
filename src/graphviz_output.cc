#include "soro/graphviz_output.h"

#include <ostream>

#include "utl/enumerate.h"

#include "soro/network.h"
#include "soro/train.h"

namespace soro {

void graphiz_output(std::ostream& out, timetable const& tt) {
  out << "digraph world {\n";
  out << R"--(
    rankdir="LR";

    node [
      fixedsize=false,
      fontname=Monospace,
      fontsize=12,
      height=2,
      width=2.5,
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
    for (auto const [i, r] : utl::enumerate(t->routes_)) {
      if (r->from_ != nullptr) {
        out << "        " << r->tag() << "\n";
      }
    }
    out << "    }\n";
  }
  for (auto const& [name, t] : tt) {
    for (auto const& r : t->routes_) {
      out << "    " << r->tag() << R"([URL="#)" << r->tag() << "\""
          << (r->from_ == nullptr ? R"(, color=darkviolet, shape=ellipse)" : "")
          << R"(, label=")" << r->train_->name_ << ": "
          << (r->from_ == nullptr ? "START" : r->from_->name_) << " -> "
          << r->to_->name_ << "\\n"
          << "sched@" << (r->from_ == nullptr ? "START" : r->from_->name_)
          << " = " << r->from_time_ << "\\n"
          << "sched@" << r->to_->name_ << " = " << r->to_time_ << "\\n";
      for (auto const& [t, speed_dpb] : r->entry_dpd_) {
        for (auto const [speed, prob] : speed_dpb) {
          out << t << " @ " << speed << "km/h"
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
          out << " [color=darkviolet, style=\"bold\"]";
        } else if (r->train_ != o->train_) {
          out << " [color=red, style=\"bold\", label=\"" << r->eotd_dpd_.first_
              << "\"]";
        }
        out << ";\n";
      }
    }
  }
  out << "}\n";
}

}  // namespace soro