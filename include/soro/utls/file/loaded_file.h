#pragma once

#include <filesystem>
#include <string>

#include "cista/hash.h"
#include "cista/mmap.h"

#include "utl/verify.h"

namespace soro::utls {

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

#if _WIN32

struct loaded_file {
  explicit loaded_file(std::filesystem::path const& p) : path_{p} {
    contents_ = read_file_to_string(p);
  }

  std::size_t size() const { return contents_.size(); }
  char const* begin() const { return contents_.data(); }
  char const* end() const { return contents_.data() + contents_.size(); }
  char const* data() const { return contents_.data(); }

  std::filesystem::path path_;
  std::string contents_;
};

#else

struct loaded_file {
  explicit loaded_file(std::filesystem::path const& p) : path_{p} {
    contents_ = cista::mmap{p.string().data(), cista::mmap::protection::READ};
  }

  std::size_t size() const { return contents_.size(); }
  uint8_t const* begin() const { return contents_.begin(); }
  uint8_t const* end() const { return contents_.end(); }
  uint8_t const* data() const { return contents_.data(); }

  std::filesystem::path path_;
  cista::mmap contents_;
};

#endif

inline loaded_file load_file(std::filesystem::path const& fp) {
  return loaded_file(fp);
}

}  // namespace soro::utls
