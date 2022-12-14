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
    cmake_path(RELATIVE_PATH path BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE relative-path)
    configure_file(${file} ${SORO_SERVER_DIR}/server_resources/${relative-path} COPYONLY)
endforeach ()


