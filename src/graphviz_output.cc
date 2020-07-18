#include "soro/graphviz_output.h"

#include <ostream>

#include "soro/network.h"
#include "soro/train.h"

namespace soro {

void graphiz_output(std::ostream& out, timetable const& tt) {
  out << "digraph world {\n";
  out << R"--(
    node [
      fixedsize=false,
      fontname=Monospace,
      fontsize=12,
      height=2,
      width=2.5
    ];
)--";
  for (auto const& [name, t] : tt) {
    out << "    {rank=same; ";
    for (auto const& r : t->routes_) {
      out << r->tag() << " ";
    }
    out << ";}\n";
  }
  for (auto const& [name, t] : tt) {
    for (auto const& r : t->routes_) {
      out << "    " << r->tag()
          << R"( [ shape=box, fixedsize=false, style = "filled, bold", label=")"
          << r->train_->name_ << ": "
          << (r->from_ == nullptr ? "START" : r->from_->name_) << " -> "
          << r->to_->name_ << "\\n"
          << "sched=" << r->from_time_ << "\\n";
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
        out << "    " << r->tag() << " -> " << o->tag() << ";\n";
      }
    }
  }
  out << "}\n";
}

}  // namespace soro