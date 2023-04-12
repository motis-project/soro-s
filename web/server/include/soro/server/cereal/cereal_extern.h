#pragma once

#include "cereal/archives/json.hpp"
#include "cereal/cereal.hpp"
#include "cereal/types/array.hpp"

#include "range/v3/range.hpp"
#include "range/v3/range/concepts.hpp"

#include "soro/utls/coordinates/gps.h"

namespace soro::utls {

template <typename Archive>
inline void CEREAL_SERIALIZE_FUNCTION_NAME(Archive& archive, gps const& gps) {
  archive(cereal::make_nvp("lon", gps.lon_), cereal::make_nvp("lat", gps.lat_));
}

template <typename Archive>
inline void CEREAL_SERIALIZE_FUNCTION_NAME(Archive& archive,
                                           bounding_box const& bb) {
  archive(cereal::make_size_tag(2));
  archive(bb.south_west_);
  archive(bb.north_east_);
}

}  // namespace soro::utls

namespace ranges {

template <typename R>
  requires(ranges::range<R>)
inline void CEREAL_SAVE_FUNCTION_NAME(cereal::JSONOutputArchive& ar, R&& r) {
  if constexpr (ranges::forward_range<R>) {
    ar(cereal::make_size_tag(static_cast<cereal::size_type>(r.size())));
  } else {  // JSONOutputArchive ignores the actual size anyways ...
    ar(cereal::make_size_tag(static_cast<cereal::size_type>(0)));
  }

  for (auto&& e : r) ar(e);
}

}  // namespace ranges

namespace cista {

template <class Archive, class T>
inline void CEREAL_SERIALIZE_FUNCTION_NAME(
    Archive& ar, soro::data::vector<T> const& vector) {
  ar(cereal::make_size_tag(static_cast<cereal::size_type>(vector.size())));
  for (auto&& v : vector) ar(v);
}

template <typename Archive>
inline std::string_view CEREAL_SAVE_MINIMAL_FUNCTION_NAME(
    Archive&, cista::basic_string<soro::data::ptr<const char>> const& str) {
  return str.view();
}

}  // namespace cista
