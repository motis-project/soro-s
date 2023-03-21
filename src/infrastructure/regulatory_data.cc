#include "soro/infrastructure/regulatory_data.h"

#include "pugixml.hpp"

#include "utl/concat.h"
#include "utl/logging.h"
#include "utl/timer.h"

#include "soro/utls/parse_fp.h"
#include "soro/utls/parse_int.h"

#include "soro/infrastructure/parsers/iss/iss_string_literals.h"
#include "soro/infrastructure/parsers/iss/parse_helpers.h"

namespace soro::infra {

regulatory_station_data parse_regulatory_stations(
    std::vector<utls::loaded_file> const& regulatory_station_files) {
  utl::scoped_timer const regulartory_timer("Parsing Regulatory Data");

  regulatory_station_data station_data;

  for (auto const& reg_file : regulatory_station_files) {
    uLOG(utl::info) << "Parsing " << reg_file.path_;

    pugi::xml_document d;
    auto success = d.load_buffer(reinterpret_cast<void const*>(reg_file.data()),
                                 reg_file.size());
    utl::verify(success, "bad xml: {}", success.description());

    for (auto const& regulatory_station : d.child(XML_ISS_DATA)
                                              .child("Ordnungsrahmen")
                                              .child("Betriebsstellen")) {
      soro::string const ds100 = regulatory_station.child_value("DS100");
      soro::string const name = regulatory_station.child_value("Name");
      station_data.ds100_to_full_name_[ds100] = name;
    }
  }

  return station_data;
}

using line_type_key = uint32_t;

static const soro::map<line_type_key, bool> HAS_SIGNALLING = {
    {1, true},  {2, true},  {3, false},  {6, true},   {8, true},
    {9, true},  {12, true}, {13, false}, {14, false}, {15, true},
    {18, true}, {20, true}, {21, true},  {23, true},  {24, false},
};

static const soro::map<line_type_key, bool> HAS_LZB = {
    {1, false},  {2, true},   {3, true},   {6, false},  {8, false},
    {9, false},  {12, false}, {13, false}, {14, false}, {15, false},
    {18, false}, {20, false}, {21, false}, {23, false}, {24, false},
};

static const soro::map<line_type_key, bool> HAS_ETCS = {
    {1, false}, {2, false},  {3, false},  {6, false},  {8, true},
    {9, true},  {12, true},  {13, true},  {14, false}, {15, false},
    {18, true}, {20, false}, {21, false}, {23, true},  {24, false},
};

soro::vector<line> parse_lines_from_file(
    utls::loaded_file const& regulatory_line_file) {
  soro::vector<line> lines;

  pugi::xml_document d;
  auto success =
      d.load_buffer(reinterpret_cast<void const*>(regulatory_line_file.data()),
                    regulatory_line_file.size());
  utl::verify(success, "Bad xml: {}", success.description());

  for (auto const& line_xml :
       d.child(XML_ISS_DATA).child("Ordnungsrahmen").child("Strecken")) {

    line l;

    l.id_ = utls::parse_int<line::id>(line_xml.child_value("Nr"));

    for (auto const line_segment_xml : line_xml.child("Streckenabschnitte")) {
      line::segment s;

      s.from_ =
          parse_kilometrage(line_segment_xml.child_value("KilometrierungVon"));
      s.to_ =
          parse_kilometrage(line_segment_xml.child_value("KilometrierungBis"));

      if (auto type = line_segment_xml.child("Art"); static_cast<bool>(type)) {
        auto const line_key = utls::parse_int<line_type_key>(
            type.attribute("Schluessel").value());

        s.signalling_ = HAS_SIGNALLING.at(line_key);
        s.lzb_ = HAS_LZB.at(line_key);
        s.etcs_ = HAS_ETCS.at(line_key);
      }

      l.segments_.push_back(s);
    }

    lines.push_back(l);
  }

  return lines;
}

soro::map<line::id, line> parse_lines(
    std::vector<utls::loaded_file> const& regulatory_line_files) {
  soro::vector<line> lines;

  for (auto const& f : regulatory_line_files) {
    utl::concat(lines, parse_lines_from_file(f));
  }

  soro::map<line::id, line> result;
  for (auto const& l : lines) {
    result.emplace(l.id_, l);
  }

  return result;
}

}  // namespace soro::infra