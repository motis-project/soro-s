#include "soro/parse_train_data.h"

#include <ostream>

#include "pugixml.hpp"

#include "utl/to_vec.h"
#include "utl/verify.h"

namespace soro {

std::ostream& operator<<(std::ostream& out, tractive_force const& f) {
  return out << f.from_ << " - " << f.to_ << ": " << f.tracitive_force_[0]
             << ", " << f.tracitive_force_[1] << ", " << f.tracitive_force_[2];
}

bool tractive_force::operator<(tractive_force const& o) const {
  return from_ < o.from_;
}

float parse_float(std::string s) {
  auto const i_comma = s.find(',');
  if (i_comma != std::string::npos) {
    s[i_comma] = '.';
  }
  try {
    return std::stof(s);
  } catch (std::exception const& e) {
    std::cout << "stof: " << e.what() << ", in=\"" << s << "\"\n";
    return 0;
  }
}

std::ostream& operator<<(std::ostream& out, train_physics const& tp) {
  out << tp.name_ << ":\n";
  out << "  max_speed=" << tp.max_speed_ << "\n";
  out << "  weight=" << tp.weight_kg_ << "\n";
  out << "  running_resistance=" << tp.running_resistance_[0] << ", "
      << tp.running_resistance_[1] << ", " << tp.running_resistance_[2] << "\n";
  for (auto const f : tp.tractive_force_) {
    out << "  " << f << "\n";
  }
  return out;
}

inline float ton_to_kg(float ton) { return ton * 1000.0F; }

std::vector<train_physics> parse_train_data(std::string const& train_spec) {
  using namespace pugi;

  xml_document d;
  auto r = d.load_buffer(reinterpret_cast<void const*>(train_spec.data()),
                         train_spec.size());
  utl::verify(r, "bad xml: {}", r.description());

  return utl::to_vec(
      d.child("Triebfahrzeugbaureihe")
          .child("Triebfahrzeugbaureihenvarianten")
          .children("Triebfahrzeugbaureihenvariante"),
      [&](auto&& variant) {
        return train_physics{
            .name_ = variant.child("Bezeichnung").child_value(),
            .weight_kg_ = ton_to_kg(
                parse_float(variant.child("EigenGewicht").child_value())),
            .max_speed_ = parse_float(
                variant.child("ZulaessigeGeschwindigkeit").child_value()),
            .running_resistance_ =
                {parse_float(
                     variant.child("Laufwiderstandsfaktor1").child_value()) /
                     1000.0F,
                 parse_float(
                     variant.child("Laufwiderstandsfaktor2").child_value()),
                 parse_float(
                     variant.child("Laufwiderstandsfaktor3").child_value())},
            .tractive_force_ = utl::to_vec(
                variant.child("Stromartausruestungen")
                    .child("Stromartausruestung")
                    .child("Zugkraftfaktoren")
                    .children("Zugkraftfaktor"),
                [&](auto&& el) {
                  return tractive_force{
                      .from_ = parse_float(
                          el.child("GeschwindigkeitVon").child_value()),
                      .to_ = parse_float(
                          el.child("GeschwindigkeitBis").child_value()),
                      .tracitive_force_ = {
                          parse_float(el.child("Faktor1").child_value()),
                          parse_float(el.child("Faktor2").child_value()),
                          parse_float(el.child("Faktor3").child_value())}};
                })};
      });
}

}  // namespace soro