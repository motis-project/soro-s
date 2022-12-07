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

    add_custom_target(soro-client-production COMMAND npm run build WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/web/client/)
    add_custom_command(TARGET soro-client-production POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/web/client/dist
            ${SORO_SERVER_DIR}/server_resources)

    add_subdirectory(web/server/)
    add_dependencies(soro-server-client soro-server)
    add_dependencies(soro-server-client soro-client-production)
endif ()