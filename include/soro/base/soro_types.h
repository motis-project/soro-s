#pragma once

#include "cista/containers/array.h"
#include "cista/containers/hash_set.h"
#include "cista/containers/variant.h"

#include "soro/utls/function_alias.h"

#if defined(SERIALIZE)

#include "cista/containers/hash_map.h"
#include "cista/containers/hash_set.h"
#include "cista/containers/ptr.h"
#include "cista/containers/tuple.h"
#include "cista/containers/unique_ptr.h"
#include "cista/containers/vector.h"

#else

#include "utl/to_vec.h"

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#endif

namespace soro {

#if defined(USE_CISTA_RAW)
namespace data = cista::raw;
#else
namespace data = cista::offset;
#endif

#if defined(SERIALIZE)

template <typename T>
using ptr = data::ptr<T const>;

template <typename T>
using non_const_ptr = data::ptr<T>;

template <typename T>
using unique_ptr = data::unique_ptr<T>;

template <typename T1, typename T2>
using pair = data::pair<T1, T2>;

template <typename... Ts>
using tuple = cista::tuple<Ts...>;

template <typename Key, typename Value>
using map = data::hash_map<Key, Value>;

template <typename T>
using set = data::hash_set<T>;

template <typename T>
using vector = data::vector<T>;

using size_type = uint32_t;

template <typename T, std::size_t Size>
using array = data::array<T, Size>;

template <typename... T>
using variant = data::variant<T...>;

using string = data::string;
using string_view = data::string_view;

template <typename T, typename... Args>
soro::unique_ptr<T> make_unique(Args&&... args) {
  return data::make_unique<T>(std::forward<Args>(args)...);
}

FUN_ALIAS(to_vec, data::to_vec)

template <typename Tuple>
constexpr inline auto tuple_size_v = cista::tuple_size_v<Tuple>;

template <std::size_t I, typename Tuple>
using tuple_element_t = cista::tuple_element_t<I, Tuple>;

#else

template <typename T>
using ptr = T const*;

template <typename T>
using non_const_ptr = T*;

template <typename T>
using unique_ptr = std::unique_ptr<T>;

template <typename T1, typename T2>
using pair = std::pair<T1, T2>;

template <typename... Ts>
using tuple = std::tuple<Ts...>;

template <typename Key, typename Value, typename Comparator = std::less<> >
using map = std::map<Key, Value, Comparator>;

template <typename T>
using set = data::hash_set<T>;

template <typename ValueType>
using vector = std::vector<ValueType>;

using size_type = size_t;

template <typename ValueType, std::size_t Size>
using array = data::array<ValueType, Size>;

template <typename... T>
using variant = data::variant<T...>;

using string = std::string;

inline const char* c_str(std::string const& s) { return s.c_str(); }

template <typename T, typename... Args>
soro::unique_ptr<T> make_unique(Args&&... args) {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

FUN_ALIAS(to_vec, utl::to_vec)

template <typename Tuple>
constexpr inline auto tuple_size_v = std::tuple_size_v<Tuple>;

template <std::size_t I, typename Tuple>
using tuple_element_t = std::tuple_element_t<I, Tuple>;

#endif

template <typename T>
constexpr bool is_pointer() {
  return std::is_pointer_v<T> || cista::is_pointer_v<T>;
}

}  // namespace soro
