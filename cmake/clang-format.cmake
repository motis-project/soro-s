find_program(CLANG_FORMAT_COMMAND NAMES clang-format clang-format-12 clang-format-11 clang-format-10 clang-format-9)
add_custom_target(format-check
        COMMAND find
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/test
        ${CMAKE_CURRENT_SOURCE_DIR}/tools
        ${CMAKE_CURRENT_SOURCE_DIR}/web
        -type f
(
        -name "*.cc"
        -o
        -name "*.h"
        )
        -print0
        | xargs -0 ${CLANG_FORMAT_COMMAND} -i
        COMMAND git status --porcelain
        COMMAND git status --porcelain | xargs -I {} -0 test -z \"{}\"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Checking source code formatting"
        VERBATIM
        )

