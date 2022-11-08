if (CMake_SOURCE_DIR STREQUAL CMake_BINARY_DIR)
    message(FATAL_ERROR "CMake_RUN_CLANG_TIDY requires an out-of-source build!")
endif ()

file(RELATIVE_PATH RELATIVE_SOURCE_DIR ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR})

string(SUBSTRING ${CMAKE_CXX_COMPILER_VERSION} 0 2 CLANG_MAJOR)
find_program(CLANG_TIDY_COMMAND NAMES clang-tidy-${CLANG_MAJOR} clang-tidy)

if (NOT CLANG_TIDY_COMMAND)
    message(FATAL_ERROR "CMake_RUN_CLANG_TIDY is ON but clang-tidy is not found!")
endif ()

set(SORO_CLANG_TIDY ${CLANG_TIDY_COMMAND})

# //TODO(julian) switch back from aNy_CasE to CamelCase
# - key:             readability-identifier-naming.TypeTemplateParameterCase
# value:           aNy_CasE

file(SHA1 ${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy.in clang_tidy_sha1)
set(CLANG_TIDY_DEFINITIONS "CLANG_TIDY_SHA1=${clang_tidy_sha1}")
unset(clang_tidy_sha1)

configure_file(.clang-tidy.in ${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy)
