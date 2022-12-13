cmake_minimum_required(VERSION 3.18)
project(soro)

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if (MSVC)
    set(TILES_WARNINGS "/W4" "/MT")
else ()
    set(TILES_WARNINGS "-Wall" "-Wextra" ${SORO_LINK_STATIC})
endif ()

file(GLOB_RECURSE soro-server-files src/*.cc)

add_library(soro-server-lib STATIC EXCLUDE_FROM_ALL ${soro-server-files})

target_compile_options(soro-server-lib PRIVATE ${TILES_WARNINGS})
target_compile_definitions(soro-server-lib PRIVATE ${SORO_COMPILE_DEFINITIONS} BOOST_BEAST_USE_STD_STRING_VIEW=1)
target_compile_features(soro-server-lib PRIVATE ${SORO_COMPILE_FEATURES})

target_include_directories(soro-server-lib PUBLIC include)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/server_resources/)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/server_resources/icons)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/server_resources/client)

# Copy the lua profiles to the binary directory
file(GLOB_RECURSE soro-server-profiles profile/*.lua)
foreach (file ${soro-server-profiles})
    set(path ${file})
    cmake_path(RELATIVE_PATH path BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE relative-path)
    configure_file(${file} ${CMAKE_BINARY_DIR}/server_resources/${relative-path} COPYONLY)
endforeach ()

# Copy the server resources to the server_resources directory in the binary directory. Includes:
#   - Icon .pngs
#   - Font .pbfs / .woff2
file(GLOB_RECURSE soro-server-resources server_resources/*.png server_resources/*.pbf server_resources/*.woff2)
foreach (file ${soro-server-resources})
    set(path ${file})
    cmake_path(RELATIVE_PATH path BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE relative-path)
    configure_file(${file} ${CMAKE_BINARY_DIR}/${relative-path} COPYONLY)
endforeach ()

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

add_executable(soro-server EXCLUDE_FROM_ALL src/server.cc)

target_compile_options(soro-server PRIVATE ${TILES_WARNINGS})
target_compile_definitions(soro-server PRIVATE ${SORO_COMPILE_DEFINITIONS})
target_compile_features(soro-server PRIVATE ${SORO_COMPILE_FEATURES})

target_link_libraries(soro-server PUBLIC soro-server-lib ${SORO_LINK_STATIC})


