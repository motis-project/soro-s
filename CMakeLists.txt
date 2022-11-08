cmake_minimum_required(VERSION 3.19)

project(soro CXX)

### --- OPTIONS --- ###

# --- CISTA OPTIONS --- #
option(SERIALIZE "Enable serialization with cista." OFF)
option(USE_CISTA_RAW "Use cista::raw instead of cista::offset." OFF)

if (SERIALIZE)
    set(SORO_COMPILE_DEFINITIONS SERIALIZE ${SORO_COMPILE_DEFINITIONS})
else ()
    set(SORO_COMPILE_DEFINITIONS STD ${SORO_COMPILE_DEFINITIONS})
endif ()

if (USE_CISTA_RAW)
    set(SORO_COMPILE_DEFINITIONS USE_CISTA_RAW ${SORO_COMPILE_DEFINITIONS})
endif ()

# --- CLANG SUITE OPTIONS --- #
option(SORO_SAN "Run clang with sanitizers." OFF)
option(SORO_LINT "Run clang-tidy with the compiler." OFF)

if (SORO_SAN)
    SET(SORO_COMPILE_DEFINITIONS SORO_SAN ${SORO_COMPILE_DEFINITIONS})
endif ()

if (SORO_LINT)
    include(cmake/clang-tidy.cmake)
endif ()

# --- MISC OPTIONS --- #
option(SORO_DEBUG "Build the code with debug information." OFF)
option(SORO_CUDA "Enable GPU-accelerated computing." OFF)

if (SORO_DEBUG)
    set(SORO_COMPILE_DEFINITIONS SORO_DEBUG ${SORO_COMPILE_DEFINITIONS})
endif ()

if (SORO_CUDA)
    set(SORO_COMPILE_DEFINITIONS SORO_CUDA ${SORO_COMPILE_DEFINITIONS})

    set(SORO_CUDA_ARCH "-gencode arch=compute_75,code=sm_75 -gencode arch=compute_61,code=sm_61")
    if (MSVC)
        set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} ${SORO_CUDA_ARCH} -lcudadevrt")
    else ()
        set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} ${SORO_CUDA_ARCH} --compiler-options -static")
    endif ()
    enable_language(CUDA)
endif ()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

### --- DEPENDENCIES --- ###

# Gets us all dependencies with the pkg tool
include(cmake/pkg.cmake)

include(cmake/buildcache.cmake)
include(cmake/clang-format.cmake)

set(CMAKE_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

### --- CXX FLAGS --- ###

set(SORO_COMPILE_OPTIONS "")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(SORO_COMPILE_OPTIONS ${SORO_COMPILE_OPTIONS}
            #            -fno-strict-aliasing
            -Weverything
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wno-newline-eof
            -Wno-missing-prototypes
            -Wno-padded
            -Wno-double-promotion
            -Wno-undef
            -Wno-undefined-reinterpret-cast
            -Wno-float-conversion
            -Wno-global-constructors
            -Wno-exit-time-destructors
            -Wno-switch-enum
            -Wno-c99-designator
            -Wno-zero-as-null-pointer-constant
            -Wno-missing-noreturn
            -Wno-deprecated-experimental-coroutine
            -Werror
            )

    if (CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 13)
        set(SORO_COMPILE_OPTIONS ${SORO_COMPILE_OPTIONS}
                -Wno-reserved-identifier)
    endif ()

    if (SORO_SAN)
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address,undefined -fno-omit-frame-pointer")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined, -fno-omit-frame-pointer")
    endif ()
elseif (MSVC)
    set(SORO_COMPILE_OPTIONS ${SORO_COMPILE_OPTIONS}
            /WX
            )
else ()
    set(SORO_COMPILE_OPTIONS ${SORO_COMPILE_OPTIONS}
            -Wall
            -Wextra
            -Werror
            -fcoroutines
            )
endif ()

set(SORO_COMPILE_FEATURES cxx_std_20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Gets us the soro-lib library
include(cmake/soro-lib.cmake)

# Gets us the soro-server-client target
include(cmake/soro-server-client.cmake)

# Generate file_paths.h for locating the test resources
configure_file(test/include/test/file_paths.h.in
        ${CMAKE_CURRENT_SOURCE_DIR}/test/include/test/file_paths.h)

# Build the tests by getting all test files and linking to doctest and soro-lib
file(GLOB_RECURSE soro-test-files test/src/*.cc web/server/test/src/*.cc)
add_executable(soro-test ${soro-test-files})

target_compile_options(soro-test PRIVATE ${SORO_COMPILE_OPTIONS})
target_compile_features(soro-test PRIVATE ${SORO_COMPILE_FEATURES})
target_compile_definitions(soro-test PRIVATE ${SORO_COMPILE_DEFINITIONS})

target_link_libraries(soro-test PUBLIC doctest soro-lib soro-server-lib)
target_include_directories(soro-test PUBLIC test/include)

# Make clang-tidy only output on soro-s files, not on dependencies.

# returns all targets except targets defined in deps/-
function(get_all_targets _result _dir)
    get_property(_subdirs DIRECTORY "${_dir}" PROPERTY SUBDIRECTORIES)
    foreach (_subdir IN LISTS _subdirs)
        if (NOT ${_subdir} MATCHES ".*deps.*")
            get_all_targets(${_result} "${_subdir}")
        endif ()
    endforeach ()

    get_directory_property(_sub_targets DIRECTORY "${_dir}" BUILDSYSTEM_TARGETS)
    set(${_result} ${${_result}} ${_sub_targets} PARENT_SCOPE)
endfunction()

# if linting is on set the clang tidy property on all defined targets
if (SORO_LINT)
    get_all_targets(all-targets ${CMAKE_CURRENT_SOURCE_DIR})

    foreach (target ${all-targets})
        set_target_properties(${target} PROPERTIES
                CXX_CLANG_TIDY ${SORO_CLANG_TIDY}
                )
    endforeach ()
endif ()

# add an ALL target for CLion convenience
add_custom_target(ALL)
add_dependencies(ALL
        soro-test
        soro-server-client)
