cmake_minimum_required(VERSION 3.18)
project(soro)

add_custom_target(soro-client COMMAND npm run build WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_command(TARGET soro-client POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/dist
        ${SORO_SERVER_DIR}/server_resources)
