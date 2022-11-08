#pragma once

#include <filesystem>
#include <fstream>
#include <vector>

namespace soro::utls {

inline void write_binary_buffer_to_file(std::filesystem::path const& fp,
                                        std::vector<unsigned char> const& buf) {
  std::fstream out_file(fp, std::fstream::out | std::fstream::binary);
  out_file.write(reinterpret_cast<char const*>(buf.data()),
                 static_cast<std::streamsize>(buf.size()));
  out_file.close();
}

}  // namespace soro::utls
