#include "soro/infrastructure/parsers/iss/iss_files.h"

#include "pugixml.hpp"

#include "utl/logging.h"

#include "soro/utls/file/file_to_binary_buffer.h"

namespace fs = std::filesystem;

namespace soro::infra {

void add_iss_file(iss_files& iss_fs, std::filesystem::path const& fp) {

  if (fp.filename().string().starts_with(RAIL_PLAN_STATIONS)) {
    iss_fs.rail_plan_files_.emplace_back(fp);
  } else if (fp.filename().string().starts_with("Stammdaten")) {
    iss_fs.core_data_files_.emplace_back(fp);
  } else if (fp.filename().string().starts_with("Ordnungsrahmen")) {

    if (fp.filename().string().contains("ORBtrst")) {
      iss_fs.regulatory_station_files_.emplace_back(fp);
    }
    if (fp.filename().string().contains("ORStr")) {
      iss_fs.regulatory_line_files_.emplace_back(fp);
    }

  } else if (fp.filename().string().starts_with("BaubetrieblicheMassnahmen")) {
    // ignore for now
  } else {
    uLOG(utl::warn)
        << "Found file " << fp
        << " but was not able to identify to which category it belongs";
  }
}

iss_files::iss_files(std::filesystem::path const& index) : index_{index} {
  pugi::xml_document d;
  auto success = d.load_buffer(reinterpret_cast<void const*>(index_.data()),
                               index_.size());

  utl::verify(success, "bad index xml: {}", success.description());

  for (auto const& file : d.child(XML_ISS_INDEX).child(FILES).children(FILE)) {
    auto const& file_name = file.child_value(NAME);
    auto const path = index.parent_path() / fs::path(file_name);

    utl::verify(exists(path), "File " + path.string() + " does not exist");

    add_iss_file(*this, path);
  }
}

iss_files get_iss_files(std::filesystem::path const& fp) {
  utl::verify(exists(fp), "Path " + fp.string() + " does not exist");

  if (fp.extension() == ".iss") {
    throw utl::fail("archives are currently not supported.");
  } else if (fp.filename().string() == "Index.xml") {
    return iss_files(fp.parent_path());
  } else if (is_directory(fp) && exists(fp / "Index.xml")) {
    return iss_files(fp / "Index.xml");
  } else if (is_directory(fp) && !exists(fp / "Index.xml")) {
    throw utl::fail("could not find Index.xml in {}", fp);
  } else {
    throw utl::fail("please specify a valid infrastructure path, got {}", fp);
  }
}

}  // namespace soro::infra