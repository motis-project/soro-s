#pragma once

#include <filesystem>
#include <fstream>

namespace soro::utls {

inline std::vector<unsigned char> read_file_to_binary_buffer(
    std::filesystem::path const& fp) {
  std::fstream iss_file(fp, std::fstream::in | std::fstream::binary);
  std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(iss_file),
                                    {});
  iss_file.close();
  return buffer;
}

}  // namespace soro::utls