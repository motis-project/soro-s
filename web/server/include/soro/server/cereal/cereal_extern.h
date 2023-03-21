#pragma once

#include "cereal/archives/json.hpp"
#include "cereal/cereal.hpp"

#include "range/v3/range.hpp"
#include "range/v3/range/concepts.hpp"

#include "soro/utls/coordinates/gps.h"

namespace soro::utls {

template <typename Archive>
inline void CEREAL_SERIALIZE_FUNCTION_NAME(Archive& archive, gps const& gps) {
  archive(cereal::make_nvp("lon", gps.lon_), cereal::make_nvp("lat", gps.lat_));
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

//! Serialization for non-arithmetic vector types
template <class Archive, class T>
inline typename std::enable_if<(!cereal::traits::is_output_serializable<
                                    cereal::BinaryData<T>, Archive>::value ||
                                !std::is_arithmetic<T>::value) &&
                                   !std::is_same<T, bool>::value,
                               void>::type
CEREAL_SERIALIZE_FUNCTION_NAME(Archive& ar,
                               soro::data::vector<T> const& vector) {
  ar(cereal::make_size_tag(
      static_cast<cereal::size_type>(vector.size())));  // number of elements
  for (auto&& v : vector) ar(v);
}

template <typename Archive>
inline std::string_view CEREAL_SAVE_MINIMAL_FUNCTION_NAME(
    Archive&, basic_string<soro::data::ptr<const char>> const& str) {
  return str.view();
}

}  // namespace cista
