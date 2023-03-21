#pragma once

#include <array>
#include <string>

namespace soro::infra {

using type_id = uint8_t;

// this enum is sorted, watch out when inserting a new entry
enum class type : type_id {
  // end elements
  BUMPER,
  TRACK_END,
  // simple elements
  KM_JUMP,
  BORDER,
  LINE_SWITCH,
  // simple switch
  SIMPLE_SWITCH,
  // cross
  CROSS,
  // directed track elements
  MAIN_SIGNAL,
  PROTECTION_SIGNAL,
  APPROACH_SIGNAL,
  RUNTIME_CHECKPOINT,
  EOTD,
  SPEED_LIMIT,
  POINT_SPEED,
  BRAKE_PATH,
  LZB_START,
  LZB_END,
  LZB_BLOCK_SIGN,
  ETCS_START,
  ETCS_END,
  ETCS_BLOCK_SIGN,
  FORCED_HALT,
  META,
  HALT,
  // undirected track elements
  TUNNEL,
  ENTRY,
  TRACK_NAME,
  RUNTIME_CHECKPOINT_UNDIRECTED,
  LEVEL_CROSSING,
  SLOPE,
  //
  INVALID
};

constexpr const auto type_count = static_cast<type_id>(type::INVALID);

constexpr type_id type_to_id(type const t) { return static_cast<type_id>(t); }
constexpr type id_to_type(type_id const id) { return static_cast<type>(id); }

constexpr std::array<type, type_count> all_types() {
  std::array<type, type_count> all_types{};

  for (type_id tid = 0; tid != type_to_id(type::INVALID); ++tid) {
    all_types[tid] = static_cast<type>(tid);
  }

  return all_types;
}

static_assert(type_to_id(type::BUMPER) == 0);
static_assert(all_types().size() == type_count);

type get_type(char const* str);
type get_type(std::string const& str);
std::string get_type_str(type const& t);

constexpr bool is_end_element(type const type) {
  return type >= type::BUMPER && type <= type::TRACK_END;
}

constexpr bool is_simple_element(type const type) {
  return type >= type::KM_JUMP && type <= type::LINE_SWITCH;
}

constexpr bool is_simple_switch(type const type) {
  return type == type::SIMPLE_SWITCH;
}

constexpr bool is_track_element(type const type) {
  return type >= type::MAIN_SIGNAL && type <= type::SLOPE;
}

constexpr bool is_directed_track_element(type const type) {
  return type >= type::MAIN_SIGNAL && type <= type::HALT;
}

constexpr bool is_undirected_track_element(type const type) {
  return type >= type::TUNNEL && type <= type::SLOPE;
}

constexpr bool is_section_element(type const type) {
  return !is_track_element(type);
}

constexpr bool is_cross(type const type) { return type == type::CROSS; }
constexpr bool is_border(type const type) { return type == type::BORDER; }

constexpr bool is_runtime_checkpoint(type const type) {
  return type == type::RUNTIME_CHECKPOINT ||
         type == type::RUNTIME_CHECKPOINT_UNDIRECTED;
}

constexpr bool is_halt(type const type) { return type == type::HALT; }

constexpr bool is_lzb(type const type) {
  return type >= type::LZB_START && type <= type::LZB_BLOCK_SIGN;
}

constexpr bool is_etcs(type const type) {
  return type >= type::ETCS_START && type <= type::ETCS_BLOCK_SIGN;
}

}  // namespace soro::infra
