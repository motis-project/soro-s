#pragma once

#include "soro/base/soro_types.h"

#include "soro/si/units.h"

#include "soro/infrastructure/brake_path.h"

#include "soro/rolling_stock/train_physics.h"
#include "soro/rolling_stock/train_type.h"

namespace soro::infra {

// fwd declaration for speed_limit
struct node;
struct element;

struct empty {};

struct eotd {
  bool signal_{false};
};

struct main_signal {
  soro::string name_;
  soro::string type_;
  soro::size_t skip_approach_;
};

struct approach_signal {
  soro::string name_;
};

// slope in radians
using slope = si::slope;

struct halt {
  soro::string name_{};
  soro::string identifier_operational_{};
  soro::string identifier_extern_{};

  bool is_passenger_{false};
  bool is_left_{false};
  bool is_right_{false};
};

struct lzb_start {
  slope avg_slope_;
};

struct lzb_block_sign {
  soro::string name_;
  soro::string ui_name_;
  soro::string control_name_;
  si::time interlocking_time_;
};

// speed limit has three different valid combinations:
//   - speed limit point of action is HERE and length_ is not 0:
//     the speed limit is in effect from the specified point and for the
//     specified length
//   - speed limit point of action is HERE and length_ is 0:
//     the speed limit is in effect from the specified point and until the next
//     relevant speed limit
//   - speed limit point of action is LAST_SIGNAL and length_ is 0:
//     speed limit is in effect from the last signal and until the next relevant
//     speed limit

// the combination LAST_SIGNAL and length_ not 0 is not valid

struct speed_limit {
  using ptr = soro::ptr<speed_limit>;
  using optional_ptr = soro::optional<ptr>;

  enum class type : uint8_t {
    general,
    special,
    divergent,
    end_special,
    invalid
  };

  // point of activation
  enum class poa : bool { here, last_signal };

  enum class affects : uint8_t { all, conventional, ctc, invalid };

  enum class source : bool { infrastructure, station_route };

  bool from_here() const { return poa_ == poa::here; }
  bool from_last_signal() const { return poa_ == poa::last_signal; }

  bool from_infrastructure() const { return source_ == source::infrastructure; }
  bool from_station_route() const { return source_ == source::station_route; }

  bool is_general() const { return type_ == type::general; }
  bool is_special() const {
    return type_ == type::special || type_ == type::end_special;
  }
  bool is_divergent() const { return type_ == type::divergent; }

  bool begins_special() const { return type_ == type::special; }
  bool ends_special() const { return type_ == type::end_special; }

  bool affects_all() const { return affects_ == affects::all; }
  bool affects_conventional() const {
    return affects_all() || affects_ == affects::conventional;
  }
  bool affects_lzb() const { return affects_all() || affects_ == affects::ctc; }

  bool affects_only_conventional() const {
    return affects_ == affects::conventional;
  }

  bool ends(speed_limit const& other) const {
    utls::expect(
        this->ends_special(),
        "ends called with speed limit that does not end a special speed limit");
    return same_specialties(other);
  }

  bool same_specialties(speed_limit const& other) const {
    return ((!train_type_.has_value() && !other.train_type_.has_value()) ||
            (train_type_.has_value() && other.train_type_.has_value() &&
             *train_type_ == *other.train_type_)) &&
           train_series_ == other.train_series_ &&
           train_series_types_ == other.train_series_types_ &&
           tilt_technologies_ == other.tilt_technologies_ &&
           transportation_specialties_ == other.transportation_specialties_;
  }

  std::ostream& print(std::ostream& out) const {
    out << "speed limit: ";

    if (limit_.is_valid()) {
      out << si::as_km_h(limit_) << " | ";
    }

    out << "point of activation: ";
    if (from_here()) {
      std::cout << " HERE";
    } else if (from_last_signal()) {
      std::cout << " LAST MS";
    }
    out << " | ";

    out << "affects: ";
    if (affects_ == affects::all) {
      out << "all";
    } else if (affects_ == affects::conventional) {
      out << "conventional";
    } else if (affects_ == affects::ctc) {
      out << "ctc";
    }
    out << " | ";

    if (is_special()) {
      if (ends_special()) {
        out << "ends special: ";
      } else {
        out << "starts special: ";
      }

      if (train_type_.has_value()) {
        out << "train type: " << train_type_.value();
      }

      if (!train_series_.empty()) {
        out << "train series: ";
      }
      for (auto const& ts : train_series_) {
        out << "( " << ts.company_ << ", " << ts.number_ << "); ";
      }

      if (!train_series_types_.empty()) {
        out << "train series types: ";
      }
      for (auto const& tst : train_series_types_) {
        out << as_val(tst) << "; ";
      }

      if (!tilt_technologies_.empty()) {
        out << "tilt technologies: ";
      }
      for (auto const& tt : tilt_technologies_) {
        out << std::to_string(as_val(tt)) << "; ";
      }

      if (!transportation_specialties_.empty()) {
        out << "transportation specialties: ";
      }
      for (auto const& tss : transportation_specialties_) {
        out << as_val(tss) << "; ";
      }
    }

    out << "\n";

    return out;
  }

  type type_{type::invalid};
  affects affects_{affects::invalid};
  bool calculated_{false};

  poa poa_{poa::here};
  source source_{source::infrastructure};
  si::speed limit_{si::speed::invalid()};
  si::length length_{si::length::invalid()};

  // if any of these is not empty then the speed limit is special
  cista::optional<rs::train_type> train_type_;
  soro::vector<rs::train_series::key> train_series_;
  soro::vector<rs::train_series::type> train_series_types_;
  soro::vector<rs::tilt_technology> tilt_technologies_;
  soro::vector<rs::transportation_specialty> transportation_specialties_;
};

struct switch_data {
  soro::string name_;
  cista::optional<soro::string> ui_identifier_;
};

using element_data = soro::variant<empty, eotd, slope, halt, speed_limit,
                                   approach_signal, main_signal, switch_data,
                                   brake_path, lzb_start, lzb_block_sign>;

}  // namespace soro::infra
