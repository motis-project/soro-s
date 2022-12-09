# This file is responsible for building the soro-server,
# which includes the soro-client

add_custom_target(soro-server-client)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# SERVER
set(SORO_SERVER_DIR ${CMAKE_CURRENT_BINARY_DIR})
file(MAKE_DIRECTORY ${SORO_SERVER_DIR})

add_subdirectory(web/server/)
add_dependencies(soro-server-client soro-server)

# CLIENT
file(GLOB_RECURSE soro-client-files
        web/client/*.html
        web/client/*.css
        web/client/*.js
        web/client/*.ico
        web/client/*.png
        web/client/*.svg
        web/client/*.map)

foreach (file ${soro-client-files})
    set(path ${file})
    cmake_path(RELATIVE_PATH path BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/web/client/ OUTPUT_VARIABLE relative-path)
    configure_file(${file} ${SORO_SERVER_DIR}/server_resources/${relative-path} COPYONLY)
endforeach ()

configure_file(resources/misc/btrs_geo.csv ${SORO_SERVER_DIR}/server_resources/misc/btrs_geo.csv COPYONLY)
file(MAKE_DIRECTORY ${SORO_SERVER_DIR}/server_resources/resources/)
file(MAKE_DIRECTORY ${SORO_SERVER_DIR}/server_resources/resources/infrastructure)
file(MAKE_DIRECTORY ${SORO_SERVER_DIR}/server_resources/resources/timetable)
