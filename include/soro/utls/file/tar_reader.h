#pragma once

#include <algorithm>
#include <optional>
#include <string_view>
#include <vector>

#include "tar/util.h"

// TODO(julian) move this and zstd_file into utl/tar

namespace soro::utls {

inline unsigned parse_oct(std::string_view s) {
  unsigned i = 0;

  auto p = begin(s);
  auto n = s.size();
  while ((*p < '0' || *p > '7') && n > 0) {
    ++p;
    --n;
  }

  while (*p >= '0' && *p <= '7' && n > 0) {
    i *= 8;
    i += static_cast<unsigned>(*p - '0');
    ++p;
    --n;
  }

  return i;
}

inline bool is_end_of_archive(std::string_view header) {
  tar::verify(header.length() == 512);
  return std::all_of(begin(header), end(header),
                     [](auto&& c) { return c == 0; });
}

inline bool check_checksum(std::string_view p) {
  auto u = 0U;
  for (auto n = 0U; n < 512; ++n) {
    if (n < 148 || n > 155) {
      u += static_cast<unsigned char>(p[n]);
    } else {
      u += 0x20;
    }
  }
  return u == parse_oct(p.substr(148, 8));
}

inline bool is_file(std::string_view buf) {
  auto const file_type = buf[156];
  return file_type == 0 || file_type == '0';
}

inline unsigned next_multiple_512(unsigned n) {
  if (n == 0) {
    return 0;
  } else {
    unsigned a = n - 1;
    a = a >> 9;
    a = a + 1;
    return a << 9;
  }
}

template <typename Reader>
struct tar_reader {
  explicit tar_reader(Reader&& r) : reader_{std::move(r)}, next_skip_{0} {}

  std::optional<std::string_view> read() {
    while (true) {
      reader_.skip(next_skip_);
      next_skip_ = 0;

      auto const opt_header = reader_.read(512);
      tar::verify(opt_header.has_value(), "invalid end of archive");

      auto const header = *opt_header;
      if (is_end_of_archive(header)) {
        return {};
      }

      tar::verify(header.size() == 512, "invalid tar file (size < 512)");
      tar::verify(check_checksum(header), "invalid checksum");

      auto const file_size = parse_oct(header.substr(124, 12));
      auto const bytes_to_read = next_multiple_512(file_size);
      current_file_name_ = header.substr(
          0, std::min(header.find('\0'), static_cast<std::size_t>(200ULL)));
      if (is_file(header)) {
        auto const buf = reader_.read(file_size);
        tar::verify(buf.value_or("").size() == file_size,
                    "invalid file in tar");
        next_skip_ = bytes_to_read - file_size;
        return buf;
      }

      reader_.skip(bytes_to_read);
    }
  }

  float progress() const { return reader_.progress(); }

  std::string current_file_name() const { return current_file_name_; }

  Reader reader_;
  size_t next_skip_;
  std::string current_file_name_;
};

}  // namespace soro::utls
