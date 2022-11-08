#pragma once

#include <filesystem>
#include <string>

#include "cista/hash.h"

#include "utl/verify.h"

namespace soro::utls {

struct loaded_file {
  auto hash() const { return cista::hash(contents_); }

  std::filesystem::path path_;
  std::string contents_;
};

inline std::string read_file_to_string(std::filesystem::path const& fp) {
  auto close_file = [](::FILE* f) { static_cast<void>(fclose(f)); };
  auto holder = std::unique_ptr<::FILE, decltype(close_file)>(
      fopen(fp.string().c_str(), "rb"), close_file);
  if (!holder) {
    return "";
  }
  ::FILE* f = holder.get();

  auto const size = static_cast<size_t>(std::filesystem::file_size(fp));

  std::string res;
  res.resize(size);
  auto const amount_read = fread(res.data(), 1, size, f);
  utl::verify(amount_read == size,
              "Number of elements actually read ({}) does not equal amount "
              "supposed to be read ({})!",
              amount_read, size);

  return res;
}

inline loaded_file load_file(std::filesystem::path const& fp) {
  return {.path_ = fp, .contents_ = read_file_to_string(fp)};
}

}  // namespace soro::utls
