#include <fstream>
#include <iostream>

#include "utl/cmd_line_parser.h"
#include "utl/concat.h"
#include "utl/enumerate.h"

#include "soro/utls/sassert.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/si/units.h"

using namespace utl;

using namespace soro;
using namespace soro::si;
using namespace soro::infra;

namespace fs = std::filesystem;

struct config {
  cmd_line_flag<std::string, required, UTL_LONG("--infra_path"),
                UTL_DESC("path to the infrastructure directory")>
      infra_path_;

  cmd_line_flag<std::string, required, UTL_LONG("--output_path"),
                UTL_DESC("path to the output file")>
      output_path_;

  bool valid_paths() const {
    return fs::exists(infra_path_.val()) &&
           fs::is_directory(infra_path_.val()) &&
           fs::exists(output_path_.val()) &&
           fs::is_directory(output_path_.val());
  }
};

int failed_parsing() {
  std::cout << "please set a valid infrastructure directory\n\n";
  std::cout << description<config>();
  std::cout << '\n';

  return 1;
}

struct adjusted_element {
  section::id section_id_;
  element_id element_id_;
  kilometrage km_;
  std::string type_;
  std::vector<std::string> names_;
};

void write_out(std::vector<section::id> const& section_ids,
               std::filesystem::path const& out, infrastructure const& infra) {

  std::vector<adjusted_element> elements;

  for (auto const& s_id : section_ids) {
    auto const& section = infra->graph_.sections_[s_id];

    std::vector<adjusted_element> section_elements;

    for (auto const& e : section.iterate<direction::Rising>()) {
      adjusted_element adjusted_element;

      adjusted_element.element_id_ = e->id();
      adjusted_element.type_ = e->get_type_str();
      adjusted_element.section_id_ = s_id;

      if (e->is(type::SIMPLE_SWITCH)) {
        auto const data =
            infra->graph_.element_data_[e->id()].as<switch_data>();
        adjusted_element.names_.push_back(data.name_);

        if (data.ui_identifier_.has_value()) {
          adjusted_element.names_.push_back(*data.ui_identifier_);
        }
      }

      if (e->is(type::CROSS)) {
        auto const data = infra->graph_.element_data_[e->id()].as<cross_data>();

        adjusted_element.names_.push_back(data.name_start_);
        if (data.name_end_.has_value()) {
          adjusted_element.names_.push_back(*data.name_end_);
        }
      }

      if (e->is(type::MAIN_SIGNAL)) {
        auto const data =
            infra->graph_.element_data_[e->id()].as<main_signal>();
        adjusted_element.names_.push_back(data.name_);
      }

      if (e->is_track_element()) {
        adjusted_element.km_ = e->get_km(nullptr);
      }

      section_elements.push_back(adjusted_element);
    }

    section_elements.front().km_ = section.low_kilometrage();
    section_elements.back().km_ = section.high_kilometrage();

    utl::concat(elements, section_elements);
  }

  std::ofstream outfile(out);

  outfile << "section_id,element_id,km,type,ds100,station_name,name1,name2\n";

  for (auto const& element : elements) {
    auto const& station = infra->element_to_station_.at(element.element_id_);
    outfile << element.section_id_ << ',' << element.element_id_ << ','
            << as_m(element.km_) << ',' << element.type_ << ','
            << station->ds100_ << ','
            << infra->full_station_names_.at(station->id_);

    for (auto const& name : element.names_) {
      outfile << ',' << name;
    }

    utls::sassert(element.names_.size() <= 2);
    for (auto i = 0U; i < 2 - element.names_.size(); ++i) {
      outfile << ',';
    }

    outfile << '\n';
  }

  outfile.close();
}

void write_out(infrastructure const& infra, std::filesystem::path const& out) {
  std::map<line::id, std::vector<section::id>> line_to_sections;

  for (auto const& [id, section] : utl::enumerate(infra->graph_.sections_)) {
    line_to_sections[section.line_id_].push_back(id);
  }

  for (auto const& [line_id, sections] : line_to_sections) {
    write_out(sections, out / (std::to_string(line_id) + ".csv"), infra);
  }
}

int main(int argc, char const** argv) {
  config c;

  std::cout << "\n\tInfrastructure Element CSV Exporter\n\n";
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
  opts.interlocking_ = false;
  opts.exclusions_ = false;
  opts.exclusion_graph_ = false;

  infrastructure const infra(opts);

  write_out(infra, c.output_path_.val());

  return 0;
}