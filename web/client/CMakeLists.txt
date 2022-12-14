cmake_minimum_required(VERSION 3.18)
project(soro)

file(GLOB_RECURSE soro-client-files
        *.html
        *.css
        *.js
        *.ico
        *.png
        *.svg
        *.map)

foreach (file ${soro-client-files})
    set(path ${file})
    cmake_path(RELATIVE_PATH path BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/web/client/ OUTPUT_VARIABLE relative-path)
    configure_file(${file} ${SORO_SERVER_DIR}/server_resources/${relative-path} COPYONLY)
endforeach ()

configure_file(../../resources/misc/btrs_geo.csv ${SORO_SERVER_DIR}/server_resources/misc/btrs_geo.csv COPYONLY)
file(MAKE_DIRECTORY ${SORO_SERVER_DIR}/server_resources/resources/)
file(MAKE_DIRECTORY ${SORO_SERVER_DIR}/server_resources/resources/infrastructure)
file(MAKE_DIRECTORY ${SORO_SERVER_DIR}/server_resources/resources/timetable)

