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

    # CLIENT
    set(SORO_CLIENT_DIR ${CMAKE_CURRENT_BINARY_DIR}/build-client)

    file(MAKE_DIRECTORY ${SORO_CLIENT_DIR})
    # Generate the build files for soro-client
    add_custom_command(TARGET soro-server-client
            PRE_BUILD
            COMMAND ${CMAKE_COMMAND}
            -GNinja
            -S ${CMAKE_CURRENT_SOURCE_DIR}
            -B ${SORO_CLIENT_DIR}
            -DCMAKE_TOOLCHAIN_FILE=$ENV{EMSDK}upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            USES_TERMINAL
            )

    # Copy the client files from the build folder to the server_resources folder
    add_custom_command(TARGET soro-server-client
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${SORO_CLIENT_DIR}/client/
            ${SORO_SERVER_DIR}/server_resources/
            )
endif ()