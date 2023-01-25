#pragma once

#include <cstring>

#include "soro/base/soro_types.h"

namespace soro::infra {

constexpr const char* const STATION = "Betriebsstelle";

constexpr const char* const TRACK_NAME = "Gleisbezeichnung";
constexpr const char* const LEVEL_CROSSING = "Bahnuebergang";
constexpr const char* const ENTRY = "Zugang";
constexpr const char* const LINE_CLASS = "Streckenklasse";

constexpr const char* const RAIL_PLAN_SECTION = "Spurplanabschnitt";
constexpr const char* const RAIL_PLAN_SECTIONS = "Spurplanabschnitte";

constexpr const char* const LINE = "Strecke";

constexpr const char* const PUBLISH_ONLY = "nurVeroeffentlichung";

constexpr const char* const BRAKE_PATH_FALLING = "BremswegF";
constexpr const char* const BRAKE_PATH_RISING = "BremswegS";

constexpr const char* const RUNTIME_CHECKPOINT = "Fahrzeitmesspunkt";

constexpr const char* const RUNTIME_CHECKPOINT_RISING = "FahrzeitmesspunktF";
constexpr const char* const RUNTIME_CHECKPOINT_FALLING = "FahrzeitmesspunktS";

constexpr const char* const ETCS_START_RISING = "ETCSAnfangS";
constexpr const char* const ETCS_START_FALLING = "ETCSAnfangF";

constexpr const char* const ETCS_END_RISING = "ETCSEndeS";
constexpr const char* const ETCS_END_FALLING = "ETCSEndeF";

constexpr const char* const ETCS_BLOCK_SIGN_RISING = "ETCSBlockkennzeichenS";
constexpr const char* const ETCS_BLOCK_SIGN_FALLING = "ETCSBlockkennzeichenF";

constexpr const char* const LZB_START_FALLING = "LZBAnfangF";
constexpr const char* const LZB_END_FALLING = "LZBEndeF";

constexpr const char* const LZB_START_RISING = "LZBAnfangS";
constexpr const char* const LZB_END_RISING = "LZBEndeS";

constexpr const char* const LZB_BLOCK_SIGN_RISING = "LZBBlockkennzeichenS";
constexpr const char* const LZB_BLOCK_SIGN_FALLING = "LZBBlockkennzeichenF";

constexpr const char* const SPEED_LIMIT_RISING = "GeschwZulaessigS";
constexpr const char* const SPEED_LIMIT_FALLING = "GeschwZulaessigF";

constexpr const char* const POINT_SPEED_RISING = "PunktgeschwF";
constexpr const char* const POINT_SPEED_FALLING = "PunktgeschwS";

constexpr const char* const SPEED_LIMIT = "GeschwBeschraenkung";

constexpr const char* const SPECIAL_SPEED_LIMIT_END_RISING =
    "GeschwEndeSpeziellS";
constexpr const char* const SPECIAL_SPEED_LIMIT_END_FALLING =
    "GeschwEndeSpeziellF";

constexpr const char* const MAIN_SIGNAL_RISING = "HauptsignalS";
constexpr const char* const MAIN_SIGNAL_FALLING = "HauptsignalF";

constexpr const char* const PROTECTION_SIGNAL_RISING = "SchutzsignalS";
constexpr const char* const PROTECTION_SIGNAL_FALLING = "SchutzsignalF";

constexpr const char* const APPROACH_SIGNAL_RISING = "VorsignalS";
constexpr const char* const APPROACH_SIGNAL_FALLING = "VorsignalF";

constexpr const char* const SIGNAL_EOTD_RISING = "SignalZugschlussstelleS";
constexpr const char* const SIGNAL_EOTD_FALLING = "SignalZugschlussstelleF";

constexpr const char* const ROUTE_EOTD_RISING = "FstrZugschlussstelleS";
constexpr const char* const ROUTE_EOTD_FALLING = "FstrZugschlussstelleF";

constexpr const char* const HALT_FREIGHT_RISING = "HalteplatzGzS";
constexpr const char* const HALT_FREIGHT_FALLING = "HalteplatzGzF";

constexpr const char* const HALT_PASSENGER_LEFT_RISING = "HalteplatzRzLinksS";
constexpr const char* const HALT_PASSENGER_LEFT_FALLING = "HalteplatzRzLinksF";

constexpr const char* const HALT_PASSENGER_RIGHT_RISING = "HalteplatzRzRechtsS";
constexpr const char* const HALT_PASSENGER_RIGHT_FALLING =
    "HalteplatzRzRechtsF";

constexpr const char* const HALT_PASSENGER_BOTH_RISING = "HalteplatzRzBeideS";
constexpr const char* const HALT_PASSENGER_BOTH_FALLING = "HalteplatzRzBeideF";

constexpr const char* const HALT_PASSENGER_RIGHT_LEFT_RISING =
    "HalteplatzRzRechtsLinksS";
constexpr const char* const HALT_PASSENGER_RIGHT_LEFT_FALLING =
    "HalteplatzRzRechtsLinksF";

constexpr const char* const FORCED_HALT_RISING = "ZwangshaltS";
constexpr const char* const FORCED_HALT_FALLING = "ZwangshaltF";

constexpr const char* const KM_JUMP_START = "KmSprungAnfang";
constexpr const char* const KM_JUMP_END = "KmSprungEnde";

constexpr const char* const SLOPE = "Neigung";

constexpr const char* const WARNING_SIGN = "Warnfeld";

constexpr const char* const BUMPER = "Prellbock";
constexpr const char* const TRACK_END = "Gleisende";
constexpr const char* const BORDER = "Betriebsstellengrenze";

constexpr const char* const SWITCH_START = "Weichenanfang";
constexpr const char* const SWITCH_STEM = "Weichenstamm";
constexpr const char* const SWITCH_BRANCH_LEFT = "WeichenabzweigLinks";
constexpr const char* const SWITCH_BRANCH_RIGHT = "WeichenabzweigRechts";

constexpr const char* const CROSS_SWITCH_START_LEFT =
    "KreuzungsweicheAnfangLinks";
constexpr const char* const CROSS_SWITCH_START_RIGHT =
    "KreuzungsweicheAnfangRechts";
constexpr const char* const CROSS_SWITCH_END_LEFT = "KreuzungsweicheEndeLinks";
constexpr const char* const CROSS_SWITCH_END_RIGHT =
    "KreuzungsweicheEndeRechts";

constexpr const char* const CROSS_START_LEFT = "KreuzungAnfangLinks";
constexpr const char* const CROSS_START_RIGHT = "KreuzungAnfangRechts";
constexpr const char* const CROSS_END_LEFT = "KreuzungEndeLinks";
constexpr const char* const CROSS_END_RIGHT = "KreuzungEndeRechts";

constexpr const char* const PARTNER = "Partner";
constexpr const char* const PARTNER_STATION = "PartnerBtrst";
constexpr const char* const PARTNER_NODE = "Partnerknoten";

constexpr const char* const TRACK_SIGN = "Gleiskennzeichen";

constexpr const char* const NEIGHBOUR_AHEAD = "NachbarGeradeaus";
constexpr const char* const NEIGHBOUR_BRANCH = "NachbarAbzweig";

constexpr const char* const RAIL_PLAN_NODE = "Spurplanknoten";

constexpr const char* const MAIN_NODE_START = "HauptknotenAnfang";
constexpr const char* const MAIN_NODE_END = "HauptknotenEnde";

constexpr const char* const KILOMETER_POINT = "Kilometrierung";

constexpr const char* const ID = "ID";
constexpr const char* const TYPE = "Typ";

constexpr const char* const XML_ISS_DATA = "XmlIssDaten";
constexpr const char* const CREATOR = "Ersteller";
constexpr const char* const VERSION = "Version";
constexpr const char* const VALID_FROM = "GueltigAb";
constexpr const char* const VALID_UNTIL = "GueltigBis";

constexpr const char* const RAIL_PLAN_STATION = "Spurplanbetriebsstelle";
constexpr const char* const RAIL_PLAN_STATIONS = "Spurplanbetriebsstellen";

constexpr const char* const STATION_ROUTE = "Betriebsstellenfahrweg";
constexpr const char* const STATION_ROUTES = "Betriebsstellenfahrwege";

constexpr const char* const PICTURE_COORDINATES = "Bildkoordinaten";
constexpr const char* const X = "X";
constexpr const char* const Y = "Y";

constexpr const char* const PICTURE_POINT = "Bildpunkt";

constexpr const char* const LINE_SWITCH_ZERO = "Streckenwechsel0";
constexpr const char* const LINE_SWITCH_ONE = "Streckenwechsel1";

constexpr const char* const TUNNEL = "Tunnel";

constexpr const char* const XML_ISS_INDEX = "XmlIssIndex";
constexpr const char* const FILES = "Dateien";
constexpr const char* const FILE = "Datei";
constexpr const char* const NAME = "Name";

constexpr const char* const SIGNAL = "Signal";
constexpr const char* const HERE = "Hier";

constexpr const char* const ALL = "Alle";
constexpr const char* const CONVENTIONAL = "Konventionell";
constexpr const char* const LZB = "LZB";

constexpr const char* const CORE_DATA = "Stammdaten";
constexpr const char* const REGULATORY_STATIONS = "Ordnungsrahmen_ORBtrst";
constexpr const char* const REGULATORY_LINES = "Ordnungsrahmen_ORStr";

constexpr const char* const TRAIN_SERIES = "Triebfahrzeugbaureihen";

constexpr const char* const DEFAULT_VALUES = "Standardwerte";

constexpr const char* const BEGIN = "Anfang";
constexpr const char* const END = "Ende";

constexpr const char* const PASSENGER_STOP = "Reisezughalt";
constexpr const char* const FREIGHT_STOP = "Gueterzughalt";

constexpr const char* const OMITTED_NODES = "EntfallendeKnoten";
constexpr const char* const OMITTED_NODE = "EntfallenderKnoten";

constexpr const char* const ATTRIBUTES = "Attribute";

constexpr const char* const COURSE = "Verlauf";
constexpr const char* const COURSE_STEM = "Stamm";

constexpr const char* const SPEED_LIMITS = "GeschwBeschraenkungen";

constexpr const char* const NODES = "Knoten";

constexpr const char* const ROUTE_FORM_TIME = "Fahrstrassenbildezeit";
constexpr const char* const BRAKE_PATH_LENGTH = "BremswegArt";
constexpr const char* const STATIONARY_SPEED_LIMIT =
    "ZulaessigeOrtsfesteGeschwindigkeit";

// TODO(julian) rename placeholder
constexpr const char* const NO_STOP_POSSIBLE = "DurchfahrtOhneHaltMoeglich";
constexpr const char* const ELECTRIFIED = "Elektrifiziert";
constexpr const char* const WRONG_WAY_DRIVE = "Falschfahrt";
constexpr const char* const P0 = "Gleiswechselbetrieb";
constexpr const char* const P1 = "Stumpfgleis";
constexpr const char* const P2 = "AusDurchfahrt40";
constexpr const char* const P3 = "HaltImmerMoeglich";
constexpr const char* const P4 = "AutomatischErzeugt";
constexpr const char* const P5 = "UmleitungStreckenwechsel";
constexpr const char* const P6 = "HerausDurchgehendesHauptgleis";
constexpr const char* const P7 = "HineinDurchgehendesHauptgleis";

constexpr std::array<const char* const, 11> STATION_ROUTE_ATTRIBUTES = {
    NO_STOP_POSSIBLE,
    ELECTRIFIED,
    WRONG_WAY_DRIVE,
    P0,
    P1,
    P2,
    P3,
    P4,
    P5,
    P6,
    P7};

constexpr soro::array<bool, STATION_ROUTE_ATTRIBUTES.size()>
    DEFAULT_ATTRIBUTE_ARRAY = {false, false, false, false, false, false,
                               false, false, false, false, false};

constexpr size_t attribute_index(const char* const attribute) {
  size_t index = 0;
  for (auto const attribute_entry : STATION_ROUTE_ATTRIBUTES) {
    if (strcmp(attribute, attribute_entry) == 0) {
      return index;
    } else {
      ++index;
    }
  }

  return std::numeric_limits<size_t>::max();
}

}  // namespace soro::infra
