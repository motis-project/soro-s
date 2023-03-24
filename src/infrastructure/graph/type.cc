#include "soro/infrastructure/graph/type.h"

#include "utl/verify.h"

#include "soro/infrastructure/parsers/iss/iss_string_literals.h"
#include "soro/utls/string.h"

namespace soro::infra {

using namespace utls;

type get_type(const char* const str) {
  switch (str_hash(str)) {
    // simple elements
    case str_hash(KM_JUMP_END):
    case str_hash(KM_JUMP_START): return type::KM_JUMP;
    case str_hash(LINE_SWITCH_ZERO):
    case str_hash(LINE_SWITCH_ONE):
      return type::LINE_SWITCH;
      // end elements
    case str_hash(BUMPER): return type::BUMPER;
    case str_hash(BORDER): return type::BORDER;
    case str_hash(TRACK_END):
      return type::TRACK_END;
      // directed track elements
    case str_hash(APPROACH_SIGNAL_RISING):
    case str_hash(APPROACH_SIGNAL_FALLING): return type::APPROACH_SIGNAL;
    case str_hash(MAIN_SIGNAL_FALLING):
    case str_hash(MAIN_SIGNAL_RISING): return type::MAIN_SIGNAL;
    case str_hash(PROTECTION_SIGNAL_FALLING):
    case str_hash(PROTECTION_SIGNAL_RISING): return type::PROTECTION_SIGNAL;
    case str_hash(ROUTE_EOTD_FALLING):
    case str_hash(ROUTE_EOTD_RISING):
    case str_hash(SIGNAL_EOTD_FALLING):
    case str_hash(SIGNAL_EOTD_RISING): return type::EOTD;
    case str_hash(SPECIAL_SPEED_LIMIT_END_FALLING):
    case str_hash(SPECIAL_SPEED_LIMIT_END_RISING):
    case str_hash(SPEED_LIMIT_RISING):
    case str_hash(SPEED_LIMIT_FALLING): return type::SPEED_LIMIT;
    case str_hash(POINT_SPEED_RISING):
    case str_hash(POINT_SPEED_FALLING): return type::POINT_SPEED;
    case str_hash(FORCED_HALT_RISING):
    case str_hash(FORCED_HALT_FALLING): return type::FORCED_HALT;
    case str_hash(RUNTIME_CHECKPOINT_FALLING):
    case str_hash(RUNTIME_CHECKPOINT_RISING): return type::RUNTIME_CHECKPOINT;
    case str_hash(HALT_FREIGHT_FALLING):
    case str_hash(HALT_FREIGHT_RISING):
    case str_hash(HALT_PASSENGER_LEFT_RISING):
    case str_hash(HALT_PASSENGER_LEFT_FALLING):
    case str_hash(HALT_PASSENGER_RIGHT_RISING):
    case str_hash(HALT_PASSENGER_RIGHT_FALLING):
    case str_hash(HALT_PASSENGER_BOTH_RISING):
    case str_hash(HALT_PASSENGER_BOTH_FALLING):
    case str_hash(HALT_PASSENGER_RIGHT_LEFT_FALLING):
    case str_hash(HALT_PASSENGER_RIGHT_LEFT_RISING): return type::HALT;
    case str_hash(BRAKE_PATH_FALLING):
    case str_hash(BRAKE_PATH_RISING):
      return type::BRAKE_PATH;
      // undirected track elements
    case str_hash(SLOPE): return type::SLOPE;
    case str_hash(TUNNEL): return type::TUNNEL;
    case str_hash(TRACK_NAME): return type::TRACK_NAME;
    case str_hash(LEVEL_CROSSING): return type::LEVEL_CROSSING;
    case str_hash(RUNTIME_CHECKPOINT):
      return type::RUNTIME_CHECKPOINT_UNDIRECTED;
    case str_hash(ENTRY):
      return type::ENTRY;
      // switches
    case str_hash(SWITCH_START):
    case str_hash(SWITCH_STEM):
    case str_hash(SWITCH_BRANCH_LEFT):
    case str_hash(SWITCH_BRANCH_RIGHT):
      return type::SIMPLE_SWITCH;
      // crosses & cross switches
    case str_hash(CROSS_START_LEFT):
    case str_hash(CROSS_END_LEFT):
    case str_hash(CROSS_START_RIGHT):
    case str_hash(CROSS_END_RIGHT):
    case str_hash(CROSS_SWITCH_START_LEFT):
    case str_hash(CROSS_SWITCH_END_LEFT):
    case str_hash(CROSS_SWITCH_START_RIGHT):
    case str_hash(CROSS_SWITCH_END_RIGHT): return type::CROSS;
    case str_hash(LZB_START_RISING):
    case str_hash(LZB_START_FALLING): return type::LZB_START;
    case str_hash(LZB_END_RISING):
    case str_hash(LZB_END_FALLING): return type::LZB_END;
    case str_hash(LZB_BLOCK_SIGN_RISING):
    case str_hash(LZB_BLOCK_SIGN_FALLING): return type::LZB_BLOCK_SIGN;
    case str_hash(ETCS_START_RISING):
    case str_hash(ETCS_START_FALLING): return type::ETCS_START;
    case str_hash(ETCS_END_RISING):
    case str_hash(ETCS_END_FALLING): return type::ETCS_END;
    case str_hash(ETCS_BLOCK_SIGN_RISING):
    case str_hash(ETCS_BLOCK_SIGN_FALLING):
      return type::ETCS_BLOCK_SIGN;
      // ignore these  for the moment
    default:
    case str_hash(PICTURE_POINT):
    case str_hash(LINE_CLASS): return type::INVALID;
  }
}

type get_type(std::string const& str) { return get_type(str.data()); }

std::string get_type_str(type const& t) {
  switch (t) {
    case type::INVALID: return "invalid";
    case type::BUMPER: return "bumper";
    case type::BORDER: return "border";
    case type::TRACK_END: return "track_end";
    case type::SIMPLE_SWITCH: return "simple_switch";
    case type::APPROACH_SIGNAL: return "as";
    case type::MAIN_SIGNAL: return "ms";
    case type::PROTECTION_SIGNAL: return "ps";
    case type::EOTD: return "eotd";
    case type::ENTRY: return "entry";
    case type::SPEED_LIMIT: return "spl";
    case type::POINT_SPEED: return "point_speed";
    case type::BRAKE_PATH: return "brake_path";
    case type::TUNNEL: return "tunnel";
    case type::TRACK_NAME: return "track_name";
    case type::HALT: return "hlt";
    case type::FORCED_HALT: return "forced_halt";
    case type::RUNTIME_CHECKPOINT: return "rtcp";
    case type::RUNTIME_CHECKPOINT_UNDIRECTED: return "rtcp_u";
    case type::KM_JUMP: return "km_jump";
    case type::LINE_SWITCH: return "line_switch";
    case type::SLOPE: return "slope";
    case type::LEVEL_CROSSING: return "level_crossing";
    case type::CROSS: return "cross";
    case type::LZB_END: return "lzb_end";
    case type::LZB_START: return "lzb_start";
    case type::LZB_BLOCK_SIGN: return "lzb_block_sign";
    case type::ETCS_END: return "etcs_end";
    case type::ETCS_START: return "etcs_start";
    case type::ETCS_BLOCK_SIGN: return "etcs_block_sign";
    case type::META: return "meta";
  }
  throw utl::fail("No type string found in infrastructure element");
}

}  // namespace soro::infra