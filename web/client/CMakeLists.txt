cmake_minimum_required(VERSION 3.18)
project(soro)

file(GLOB_RECURSE soro-client-cxx-files embind/*.cc)
add_executable(soro-client ${soro-client-cxx-files})

target_compile_options(soro-client PRIVATE ${SORO_COMPILE_OPTIONS})
target_compile_features(soro-client PRIVATE ${SORO_COMPILE_FEATURES})
target_compile_definitions(soro-client PRIVATE ${SORO_COMPILE_DEFINITIONS})

target_include_directories(soro-client PUBLIC embind/include/)

set(CLIENT_DIR ${CMAKE_BINARY_DIR}/client/)

set_target_properties(soro-client PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CLIENT_DIR})
set_target_properties(soro-client PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CLIENT_DIR})
set_target_properties(soro-client PROPERTIES SUFFIX ".js")
set_target_properties(soro-client PROPERTIES COMPILE_FLAGS "-pthread")
set_target_properties(soro-client PROPERTIES LINK_FLAGS
        "-std=c++20 -s INITIAL_MEMORY=3900MB -s ALLOW_MEMORY_GROWTH=1 -s MAXIMUM_MEMORY=4GB -s WASM=1 -pthread -lembind -lidbfs.js")

target_link_libraries(soro-client soro-lib)

file(GLOB_RECURSE soro-client-files *.html *.css *.js *.ico *.png *.svg *.map)
foreach (file ${soro-client-files})
    set(path ${file})
    cmake_path(RELATIVE_PATH path BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE relative-path)
    configure_file(${file} ${CLIENT_DIR}${relative-path} COPYONLY)
endforeach ()

# This is a workaround to make it possible to use
# import { FS ... } from "soro-client.js"
# in javascript. Should be achievable by outputting .mjs with emscripten ...
add_custom_command(TARGET soro-client POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        -D SOURCE=${CLIENT_DIR}/soro-client.js
        -D DEST=${CLIENT_DIR}/soro-client.js
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/emscripten-fix.cmake)
