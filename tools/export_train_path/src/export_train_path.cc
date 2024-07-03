#include <fstream>
#include <iostream>

#include "utl/cmd_line_parser.h"
#include "utl/enumerate.h"

#include "soro/utls/unixtime.h"

#include "soro/base/issues.h"

#include "soro/si/units.h"

#include "soro/infrastructure/graph/type_set.h"
#include "soro/infrastructure/infrastructure.h"

#include "soro/rolling_stock/train_series.h"

#include "soro/timetable/timetable.h"
#include "soro/timetable/timetable_options.h"

#include "soro/runtime/common/interval.h"
#include "soro/runtime/euler_runtime.h"

using namespace soro;
using namespace soro::tt;
using namespace soro::infra;
using namespace utl;

namespace fs = std::filesystem;

struct config {
  cmd_line_flag<fs::path, required, UTL_LONG("--infra_path"),
                UTL_DESC("path to the infrastructure directory")>
      infra_path_;

  cmd_line_flag<fs::path, required, UTL_LONG("--timetable_path"),
                UTL_DESC("path to the timetable directory")>
      timetable_path_;

  cmd_line_flag<fs::path, required, UTL_LONG("--output_path"),
                UTL_DESC("path to the output file")>
      output_path_;

  cmd_line_flag<train::id, required, UTL_LONG("--train"),
                UTL_DESC("train id to export")>
      train_id_;

  bool valid_paths() const {
    return fs::exists(infra_path_.val()) &&
           fs::is_directory(infra_path_.val()) &&
           fs::exists(timetable_path_.val()) &&
           fs::is_directory(timetable_path_.val());
  }
};

int failed_parsing() {
  std::cout << "please set a valid infrastructure directory\n\n";
  std::cout << description<config>();
  std::cout << '\n';

  return 1;
}

std::string unpack_si(si::speed const& sp) {
  return std::to_string(static_cast<uint32_t>(si::as_km_h(sp)));
}

std::string unpack_si(si::accel const& ac) {
  return std::to_string(si::as_m_s2(ac));
}

std::string unpack_si(si::weight const& w) {
  return std::to_string(si::as_ton(w));
}

std::string unpack_si(si::length const& l) {
  return std::to_string(si::as_m(l));
}

std::string unpack_si(si::slope const& m) {
  return std::to_string(std::tan(m.val_) * 1000.0);
}

std::string unpack_si(rs::tractive_force_1_t const& t1) {
  return std::to_string(si::as_kn(t1));
}

std::string unpack_si(rs::tractive_force_2_t const& t2) {
  return std::to_string(rs::as_kn_h_per_km(t2));
}

std::string unpack_si(rs::tractive_force_3_t const& t3) {
  return std::to_string(rs::as_kn_h2_per_km2(t3));
}

std::string unpack_rolling(rs::rolling_resistance_t const& r1,
                           si::weight const& train_weight) {
  return std::to_string(rs::as_per_mille(r1, train_weight));
}

std::string unpack_dampening(rs::dampening_resistance_t const& r2,
                             si::weight const& train_weight) {
  return std::to_string(rs::as_h_km(r2, train_weight));
}

std::string unpack_drag(rs::drag_coefficient_t const& r3) {
  return std::to_string(rs::as_kg_h_km(r3));
}

std::string unpack_slope(si::slope const slope) {
  return std::to_string(static_cast<uint32_t>(std::tan(slope.val_) * 1000.0));
}

template <std::size_t Size>
std::string unpack_array(std::array<char, Size> const& a) {
  std::string result;
  auto const it = std::find(std::begin(a), std::end(a), '\0');
  std::copy(std::begin(a), it, std::back_inserter(result));
  return result;
}

std::string unpack_brake_path(brake_path_key const path_key) {
  auto const str = unpack_array(path_key);
  return std::to_string(std::stoi(str) * 100);
}

void export_train(train const& train, std::filesystem::path const& out_path,
                  infrastructure const& infra) {
  using cista::get;
  using std::get;

  std::ofstream out(out_path);

  // dump the brake tables

  for (rs::brake_type t{0}; t < infra->brake_tables_.types_.size(); ++t) {
    auto const& brake_type = infra->brake_tables_.types_[t];
    for (brake_path path{0}; path < brake_type.size(); ++path) {
      auto const& table = brake_type[path];

      if (table.empty()) continue;

      out << unpack_brake_path(infra->dictionaries_.brake_path_.key(path))
          << ' ' << unpack_array(infra->dictionaries_.brake_type_.key(t)) << ' '
          << table.lines_.size() << '\n';

      for (auto const& line : table.lines_) {
        out << unpack_slope(line.slope_) << ' ';
        for (brake_table::line::index i{0}; i < line.size(); ++i) {
          out << unpack_si(brake_table::line::get_speed(i)) << ' '
              << static_cast<uint32_t>(line.get_percentage(i).val_) << "  ";
        }

        out << '\n';
      }
    }
  }

  out << "---\n";

  out << "art: " << (train.uses_freight() ? "G" : "R") << '\n';
  out << "bremshundertstel: "
      << static_cast<uint32_t>(train.physics_.percentage().val_) << '\n';
  out << "bremsstellung: "
      << infra->dictionaries_.brake_position_.description(
             train.physics_.brake_position())
      << '\n';
  out << "bremsverz-konventionell: "
      << unpack_si(-train.physics_.train_class_.deacceleration_) << '\n';
  out << "gattung: 1337.42\n";

  auto const bearing = train.physics_.bearing_friction().val_;
  auto const air = train.physics_.air_resistance().val_;

  out << "gueterzug-c0: " << bearing * 1000.0 << '\n';
  out << "gueterzug-l: " << (air * 1000.0 / std::pow(3.6, 2) * 100.0) - 0.007
      << '\n';

  // if we want to support lzb (non-etcs ctc), we have to recheck this
  // check if necessary that all train vehicles have lzb or is it enough
  // that the train has lzb?
  utl::verify(!lzb_supported, "required for lzb support");
  out << "lzb: 0\n";

  out << "tfz-anzahl: " << train.physics_.vehicle_count() << '\n';

  for (auto const [id, tv] : utl::enumerate(train.physics_.units())) {
    auto const tv_str = "tfz" + std::to_string(id);

    out << tv_str << "-art: 1337.42\n";
    out << tv_str << "-bezeichnung: 1337.42\n";
    out << tv_str << "-dienstgewicht: 0\n";
    out << tv_str << "-eigengewicht: " << unpack_si(tv.weight_) << '\n';
    utl::verify(!train_length_supported, "required for train length");
    //    out << tv_str << "-laenge: " << unpack_si(tv.length_) << '\n';
    out << tv_str << "-laenge: " << 0.0 << '\n';
    out << tv_str << "-laufwiderstand-fak1: "
        << unpack_rolling(get<2>(tv.resistance_curve_.factors_), tv.weight_)
        << '\n';
    out << tv_str << "-laufwiderstand-fak2: "
        << unpack_dampening(get<1>(tv.resistance_curve_.factors_), tv.weight_)
        << '\n';
    out << tv_str << "-laufwiderstand-fak3: "
        << unpack_drag(get<0>(tv.resistance_curve_.factors_)) << '\n';
    out << tv_str << "-massezuschlag: " << (tv.mass_factor_.val_ - 1.0) * 100.0
        << '\n';
    out << tv_str << "-v_max: " << unpack_si(tv.max_speed_) << '\n';

    // TODO(julian) says if the vehicle is exerting force, not necessarily true
    out << tv_str << "-wirkt: 1\n";
    for (auto const [p_id, p] : utl::enumerate(tv.traction_curve().pieces_)) {
      auto const p_str = "-zk" + std::to_string(p_id + 1);
      out << tv_str << p_str << "-a: " << unpack_si(get<0>(p.piece_.factors_))
          << '\n';
      out << tv_str << p_str << "-b: " << unpack_si(get<1>(p.piece_.factors_))
          << '\n';
      out << tv_str << p_str << "-c: " << unpack_si(get<2>(p.piece_.factors_))
          << '\n';
      out << tv_str << p_str << "-vbis: " << unpack_si(p.to_) << '\n';
    }
    out << tv_str << "-zk: " << tv.traction_curve().pieces_.size() << '\n';

    // TODO(julian) whatever this does ...
    out << tv_str << "-zkkorrekturfaktor: 1\n";
  }

  out << "v_einbruch: " << unpack_si(train.start_speed_) << "\n";

  out << "v_max: " << unpack_si(train.physics_.max_speed_) << '\n';
  out << "v_max_konv: " << unpack_si(infra->defaults_.stationary_speed_limit_)
      << '\n';

  utl::verify(!lzb_supported, "required for lzb support");
  out << "v_max_lzb: 0.0\n";

  utl::verify(!train_length_supported, "required for train length");
  //  out << "wz-laenge: " << unpack_si(train.physics_.length()) << '\n';
  out << "wz-laenge: " << 0.0 << '\n';
  out << "wz-masse: " << unpack_si(train.physics_.carriage_weight()) << '\n';
  out << "wz-reisewagen: " << train.physics_.wagons() << '\n';

  out << "zuggruppe: 1337.42\n";
  out << "zugname: 1337.42\n";

  out << "---\n";
  out << "# LZB Lange Bogen Tunnel Pkt v BrW MaNeig GeNeig HaZt Name "
         "Ergebnisauswahl\n";

  auto const interval_list = runtime::get_intervals(train, type_set{{}}, infra);

  utl::verify(!lzb_supported, "required for lzb support");
  utl::verify(!tunnel_supported, "required for tunnel support");
  struct line {
    bool lzb_{false};
    si::length length_{si::length::zero()};
    double arc_{0.0};
    double tunnel_{0.0};
    bool point_{false};
    si::speed spl_{si::speed::invalid()};
    si::length brake_path_{si::from_m(1000)};
    si::slope avg_slope_{si::slope::zero()};
    si::slope current_slope_{si::slope::zero()};
    duration stop_time_{duration::zero()};
    std::string name_{"noname"};
    std::string selection_{"229"};
  };

  line l;

  for (auto const& [id, interval] : utl::enumerate(interval_list)) {
    auto const print_line = [&]() {
      out << (l.lzb_ ? "1" : "0") << " " << unpack_si(l.length_) << " "
          << l.arc_ << " " << l.tunnel_ << " " << (l.point_ ? "1" : "0") << " "
          << unpack_si(l.spl_) << " " << unpack_si(l.brake_path_) << " "
          << unpack_si(l.avg_slope_) << " " << unpack_si(l.current_slope_)
          << " " << l.stop_time_.count() << " " << l.name_ << " "
          << l.selection_ << '\n';
    };

    l.point_ = false;
    l.stop_time_ = duration::zero();
    l.spl_ = interval.p1_->limit_;
    l.name_ = "interval-" + std::to_string(id);
    l.length_ = interval.length();
    l.current_slope_ = interval.slope();
    l.avg_slope_ = interval.avg_slope();
    l.brake_path_ = *get_brake_path_length(interval.brake_path(), infra);
    print_line();

    if (interval.ends_on_halt()) {
      l.point_ = true;
      l.stop_time_ = (*interval.sequence_point())->stop_time();
      l.spl_ = si::speed::zero();
      l.name_ += "-halt";
      l.length_ = si::length::zero();
      l.current_slope_ = interval.slope();
      print_line();
    }
  }

  out.close();
}

int main(int argc, char const** argv) {
  config c;

  std::cout << "\n\tTrain Path Exporter\n\n";
  try {
    c = parse<struct config>(argc, argv);
  } catch (...) {
    return failed_parsing();
  }

  if (!c.valid_paths()) {
    return failed_parsing();
  }

  auto opts = make_infra_opts(c.infra_path_.val(), "");
  opts.layout_ = false;
  opts.interlocking_ = true;
  opts.exclusions_ = false;
  opts.exclusion_sets_ = false;

  auto const tt_opts = make_timetable_opts(c.timetable_path_.val());

  infrastructure const infra(opts);
  timetable tt(tt_opts, infra);

  auto const& train = tt->trains_[c.train_id_.val()];

  export_train(train, c.output_path_.val(), infra);

  auto const results = runtime::euler::runtime_calculation(
      train, infra, {type::HALT, type::RUNTIME_CHECKPOINT_UNDIRECTED});

  std::cout << (results.times_.back().arrival_ -
                results.times_.front().departure_)
                   .count()
            << '\n';

  for (auto const& [id, t] : utl::enumerate(results.times_)) {
    auto const time_since_start = [&](relative_time const rt) {
      return (rt - results.times_.front().departure_).count();
    };
    std::cout << id << " " << time_since_start(t.arrival_) << '\n';
    std::cout << id << " " << time_since_start(t.departure_) << '\n';
  }

  return 0;
}