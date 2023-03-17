cmake_minimum_required(VERSION 3.18)
project(soro)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

file(GLOB_RECURSE soro-server-files src/*.cc)

add_library(soro-server-lib STATIC EXCLUDE_FROM_ALL ${soro-server-files})

target_compile_options(soro-server-lib PRIVATE ${SORO_COMPILE_OPTIONS})
target_compile_features(soro-server-lib PRIVATE ${SORO_COMPILE_FEATURES})
target_compile_definitions(soro-server-lib PRIVATE ${SORO_COMPILE_DEFINITIONS} BOOST_BEAST_USE_STD_STRING_VIEW=1)

target_include_directories(soro-server-lib PUBLIC include)

file(MAKE_DIRECTORY ${SORO_SERVER_DIR}/server_resources/)
file(MAKE_DIRECTORY ${SORO_SERVER_DIR}/server_resources/icons)

# Copy the lua profiles to the binary directory
file(GLOB_RECURSE soro-server-profiles profile/*.lua)
foreach (file ${soro-server-profiles})
    set(path ${file})
    cmake_path(RELATIVE_PATH path BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE relative-path)
    configure_file(${file} ${SORO_SERVER_DIR}/server_resources/${relative-path} COPYONLY)
endforeach ()

# Copy the server resources to the server_resources directory in the binary directory. Includes:
#   - Icon .pngs
#   - Font .pbfs / .woff2
file(GLOB_RECURSE soro-server-resources server_resources/*.png server_resources/*.pbf server_resources/*.woff2)
foreach (file ${soro-server-resources})
    set(path ${file})
    cmake_path(RELATIVE_PATH path BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE relative-path)
    configure_file(${file} ${SORO_SERVER_DIR}/${relative-path} COPYONLY)
endforeach ()

configure_file(${CMAKE_SOURCE_DIR}/resources/misc/btrs_geo.csv ${SORO_SERVER_DIR}/server_resources/misc/btrs_geo.csv COPYONLY)
file(MAKE_DIRECTORY ${SORO_SERVER_DIR}/server_resources/resources/)
file(GLOB_RECURSE soro-resources ${CMAKE_SOURCE_DIR}/resources/*.xml ${CMAKE_SOURCE_DIR}/resources/*.osm ${CMAKE_SOURCE_DIR}/resources/*.txt ${CMAKE_SOURCE_DIR}/resources/search_indices/*)
foreach (file ${soro-resources})
    set(path ${file})
    cmake_path(RELATIVE_PATH path BASE_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_VARIABLE relative-path)
    configure_file(${file} ${SORO_SERVER_DIR}/server_resources/${relative-path} COPYONLY)
endforeach ()

# Disable warnings for tiles dependency by configuring it as a systems dependency
set_target_properties(tiles PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:tiles,INTERFACE_INCLUDE_DIRECTORIES>)

target_link_libraries(soro-server-lib PUBLIC
        pugixml
        rapidjson
        utl
        tiles
        tiles-import-library
        boost
        soro-lib
        ${SORO_LINK_STATIC}
        )

add_executable(soro-server src/server.cc)
set_target_properties(soro-server PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${SORO_SERVER_DIR})

target_compile_options(soro-server PRIVATE ${SORO_COMPILE_OPTIONS})
target_compile_features(soro-server PRIVATE ${SORO_COMPILE_FEATURES})
target_compile_definitions(soro-server PRIVATE ${SORO_COMPILE_DEFINITIONS})

target_link_libraries(soro-server PUBLIC soro-server-lib)

