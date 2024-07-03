#pragma once

#include <string>

namespace soro::utls {

// TODO(julian) this is a placeholder for a small_vector implementation

#if defined(SERIALIZE)

#if defined(USE_CISTA_RAW)
namespace data = cista::raw;
#else
namespace data = cista::offset;
#endif

template <typename T>
using small_vector = data::vector<T>;

#else

template <typename T>
using small_vector = std::basic_string<T>;

#endif

}  // namespace soro::utls
