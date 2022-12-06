# This file is responsible for building the soro-server,
# which includes the soro-client build with emscripten

# For this to work it is necessary that the environment variable
#   EMSDK=/path/to/emsdk/
# is set.

add_custom_target(soro-server-client)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

if (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    # Gets us the soro-client interface (using emscripten)
    add_subdirectory(web/client)
else ()
    # SERVER
    set(SORO_SERVER_DIR ${CMAKE_CURRENT_BINARY_DIR})
    file(MAKE_DIRECTORY ${SORO_SERVER_DIR})

    add_subdirectory(web/server/)
    add_dependencies(soro-server-client soro-server)

    file(GLOB_RECURSE soro-client-files web/client/dist/ *.html *.css *.js *.ico *.png *.svg *.map)
    foreach (file ${soro-client-files})
        set(path ${file})
        cmake_path(RELATIVE_PATH path BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/web/client/dist OUTPUT_VARIABLE relative-path)
        configure_file(${file} ${SORO_SERVER_DIR}/server_resources/${relative-path} COPYONLY)
    endforeach ()
endif ()