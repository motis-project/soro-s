cmake_minimum_required(VERSION 3.18)
project(soro)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

file(GLOB_RECURSE soro-server-files src/*.cc)

add_library(soro-server-lib STATIC EXCLUDE_FROM_ALL ${soro-server-files})

# --- compile options

set(SORO_SERVER_COMPILE_OPTIONS ${SORO_COMPILE_OPTIONS})

# --- compile features

set(SORO_SERVER_COMPILE_FEATURES ${SORO_COMPILE_FEATURES})

# --- compile definitions

set(SORO_SERVER_COMPILE_DEFINITIONS ${SORO_COMPILE_DEFINITIONS})

# rename the cereal serialization function to avoid naming conflicts with cista
list(APPEND SORO_SERVER_COMPILE_DEFINITIONS CEREAL_SERIALIZE_FUNCTION_NAME=cereal_serialize)

# --- set target options/features/definitions

target_compile_options(soro-server-lib PRIVATE ${SORO_SERVER_COMPILE_OPTIONS})
target_compile_features(soro-server-lib PRIVATE ${SORO_SERVER_COMPILE_FEATURES})
target_compile_definitions(soro-server-lib PRIVATE ${SORO_SERVER_COMPILE_DEFINITIONS})

target_include_directories(soro-server-lib PUBLIC include)

# Copy the server resources to the server_resources directory in the binary directory. Includes:
#   - Icon .png
#   - Font .pbf
#   - OSM profiles .lua
#   - Favicon .ico
#   - DS100 -> GPS mapping .csv
file(GLOB_RECURSE soro-server-resources server_resources/*[.png,.pbf,.lua,.ico,.csv])
foreach(file ${soro-server-resources})
  set(path ${file})
  cmake_path(RELATIVE_PATH path BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE relative-path)
  configure_file(${file} ${SORO_SERVER_DIR}/${relative-path} COPYONLY)
endforeach()

# Disable warnings for tiles dependency by configuring it as a systems dependency
set_target_properties(tiles PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:tiles,INTERFACE_INCLUDE_DIRECTORIES>)
set_target_properties(guess PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:guess,INTERFACE_INCLUDE_DIRECTORIES>)

target_link_libraries(soro-server-lib PUBLIC
  guess
  pugixml
  rapidjson
  utl
  tiles
  tiles-import-library
  boost
  web-server
  soro-lib
  cereal
  range-v3
  ${SORO_LINK_STATIC}
)

add_executable(soro-server src/main.cc)
set_target_properties(soro-server PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${SORO_SERVER_DIR})

target_compile_options(soro-server PRIVATE ${SORO_COMPILE_OPTIONS})
target_compile_features(soro-server PRIVATE ${SORO_COMPILE_FEATURES})
target_compile_definitions(soro-server PRIVATE ${SORO_COMPILE_DEFINITIONS})

target_link_libraries(soro-server PUBLIC soro-server-lib)

