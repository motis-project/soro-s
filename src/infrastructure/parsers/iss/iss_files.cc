#include "soro/infrastructure/parsers/iss/iss_files.h"

#include "pugixml.hpp"

#include "utl/logging.h"
#include "utl/timer.h"

#include "soro/utls/file/file_to_binary_buffer.h"
#include "soro/utls/file/tar_reader.h"
#include "soro/utls/file/zstd_file.h"

namespace fs = std::filesystem;

namespace soro::infra {

void add_iss_file(iss_files& iss_fs, std::filesystem::path const& fp,
                  std::string&& contents) {
  if (fp.filename().string().starts_with(RAIL_PLAN_STATIONS)) {
    iss_fs.rail_plan_files_.push_back({.path_ = fp, .contents_ = contents});
  } else if (fp.filename().string().starts_with("Stammdaten")) {
    iss_fs.core_data_files_.push_back({.path_ = fp, .contents_ = contents});
  } else if (fp.filename().string().starts_with("Ordnungsrahmen_ORBtrst")) {
    iss_fs.regulatory_station_files_.push_back(
        {.path_ = fp, .contents_ = contents});
  } else if (fp.filename().string().starts_with("Ordnungsrahmen_ORStr")) {
    iss_fs.regulatory_line_files_.push_back(
        {.path_ = fp, .contents_ = contents});
  } else if (fp.filename().string().starts_with("BaubetrieblicheMassnahmen")) {
  } else {
    uLOG(utl::warn)
        << "Found file " << fp
        << " but was not able to identify to which category it belongs";
  }
}

iss_files iss_archive_to_iss_files(unsigned char const* data,
                                   size_t const size) {
  utl::scoped_timer const parse_timer("Unpacking ISS archive");
  iss_files files;

  utls::tar_reader<zstd_file> r{zstd_file{data, size}};

  std::optional<std::string> s;
  while ((s = r.read())) {
    add_iss_file(files, r.current_file_name(), std::move(*s));
  }

  return files;
}

iss_files index_to_iss_files(fs::path const& fp) {
  iss_files files;

  auto const& index_string = utls::read_file_to_string(fp);

  pugi::xml_document d;
  auto success = d.load_buffer(
      reinterpret_cast<void const*>(index_string.data()), index_string.size());
  utl::verify(success, "bad xml: {}", success.description());

  for (auto const& file : d.child(XML_ISS_INDEX).child(FILES).children(FILE)) {
    auto const& file_name = file.child_value(NAME);
    auto const path = fp.parent_path() / fs::path(file_name);

    utl::verify(exists(path), "File " + path.string() + " does not exist");

    add_iss_file(files, path, utls::read_file_to_string(path));
  }

  return files;
}

iss_files directory_to_iss_files(std::filesystem::path const& fp) {
  iss_files files;

  for (auto const& f : std::filesystem::directory_iterator{fp}) {
    add_iss_file(files, f.path(), utls::read_file_to_string(f));
  }

  return files;
}

iss_files get_iss_files(std::filesystem::path const& fp) {
  utl::verify(exists(fp), "Path " + fp.string() + " does not exist");

  if (fp.extension() == ".iss") {
    auto const& buffer = utls::read_file_to_binary_buffer(fp);
    return iss_archive_to_iss_files(&buffer.front(), buffer.size());
  } else if (fp.filename().string() == "Index.xml") {
    return index_to_iss_files(fp);
  } else if (fp.extension() == ".xml") {
    iss_files files;
    files.rail_plan_files_ = {
        {.path_ = fp, .contents_ = utls::read_file_to_string(fp)}};
    return files;
  } else if (is_directory(fp) && exists(fp / "Index.xml")) {
    return index_to_iss_files(fp / "Index.xml");
  } else if (is_directory(fp) && !exists(fp / "Index.xml")) {
    return directory_to_iss_files(fp);
  } else {
    throw utl::fail("Please specify a valid infrastructure path");
  }
}

}  // namespace soro::infra