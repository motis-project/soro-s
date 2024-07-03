#include "soro/infrastructure/parsers/iss/iss_files.h"

#include <cstddef>
#include <filesystem>
#include <vector>

#include "pugixml.hpp"

#include "utl/logging.h"
#include "utl/timer.h"
#include "utl/verify.h"

#include "soro/utls/parallel_for.h"

#include "soro/infrastructure/parsers/iss/iss_string_literals.h"

namespace soro::infra {

using namespace pugi;

namespace fs = std::filesystem;

pugi::xml_document parse_xml(fs::path const& fp) {
  pugi::xml_document d;
  auto const success = d.load_file(fp.c_str());
  utl::verify(success, "bad {} xml: {}", fp, success.description());
  return d;
}

std::vector<pugi::xml_document> parse_xmls(std::vector<fs::path> const& fps) {
  std::vector<pugi::xml_document> result(fps.size());

  utls::parallel_for(fps.size(), [&](std::size_t const job_idx) {
    result[job_idx] = parse_xml(fps[job_idx]);
  });

  return result;
}

struct iss_filepaths {
  explicit iss_filepaths(fs::path const& index_fp,
                         xml_document const& index_xml) {
    for (auto const file :
         index_xml.child(XML_ISS_INDEX).child(FILES).children(FILE)) {
      auto const& file_name = file.child_value(NAME);
      auto const path = index_fp.parent_path() / fs::path(file_name);

      utl::verify(exists(path), "file " + path.string() + " does not exist");

      add_iss_file(path);
    }
  }

  void add_iss_file(std::filesystem::path const& fp) {
    if (fp.filename().string().starts_with(RAIL_PLAN_STATIONS)) {
      rail_plan_files_.emplace_back(fp);
    } else if (fp.filename().string().starts_with("Stammdaten")) {
      core_data_files_.emplace_back(fp);
    } else if (fp.filename().string().starts_with("Ordnungsrahmen")) {

      if (fp.filename().string().contains("ORBtrst")) {
        regulatory_station_files_.emplace_back(fp);
      }
      if (fp.filename().string().contains("ORStr")) {
        regulatory_line_files_.emplace_back(fp);
      }

    } else if (fp.filename().string().starts_with(
                   "BaubetrieblicheMassnahmen")) {
      construction_work_files_.emplace_back(fp);
    } else {
      uLOG(utl::warn)
          << "Found file " << fp
          << " but was not able to identify to which category it belongs";
    }
  }

  std::vector<std::filesystem::path> rail_plan_files_;
  std::vector<std::filesystem::path> core_data_files_;
  std::vector<std::filesystem::path> regulatory_station_files_;
  std::vector<std::filesystem::path> regulatory_line_files_;
  std::vector<std::filesystem::path> construction_work_files_;
};

std::filesystem::path get_index_fp(std::filesystem::path const& fp) {
  utl::verify(exists(fp), "Path " + fp.string() + " does not exist");

  if (fp.filename().string() == "Index.xml") {
    return fp;
  } else if (is_directory(fp) && exists(fp / "Index.xml")) {
    return fp / "Index.xml";
  } else if (is_directory(fp) && !exists(fp / "Index.xml")) {
    throw utl::fail("could not find Index.xml in {}", fp);
  } else {
    throw utl::fail("please specify a valid infrastructure path, got {}", fp);
  }
}

iss_files::iss_files(std::filesystem::path const& fp) {
  utl::scoped_timer const timer("loading and parsing xml files");

  auto const index_fp = get_index_fp(fp);
  index_ = parse_xml(index_fp);

  iss_filepaths const iss_fps(index_fp, index_);

  rail_plan_files_ = parse_xmls(iss_fps.rail_plan_files_);
  core_data_files_ = parse_xmls(iss_fps.core_data_files_);
  regulatory_line_files_ = parse_xmls(iss_fps.regulatory_line_files_);
  regulatory_station_files_ = parse_xmls(iss_fps.regulatory_station_files_);
  construction_work_files_ = parse_xmls(iss_fps.construction_work_files_);
}

}  // namespace soro::infra