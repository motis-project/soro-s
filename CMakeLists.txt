cmake_minimum_required(VERSION 3.17)
project(soro)

option(RAPID_LINT "Run clang-tidy with the compiler." OFF)

include(cmake/buildcache.cmake)
include(cmake/pkg.cmake)
if(RAPID_LINT)
  include(cmake/clang-tidy.cmake)
endif()

file(GLOB_RECURSE soro-lib-files src/*.cc)
add_library(soro-lib ${soro-lib-files})
target_link_libraries(soro-lib utl cista date)
target_compile_features(soro-lib PUBLIC cxx_std_20)
target_include_directories(soro-lib PUBLIC include)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  target_compile_options(soro-lib PRIVATE
    -fno-strict-aliasing
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
    -Werror
  )
elseif(MSVC)
  target_compile_options(soro-lib PRIVATE
    /WX
  )
else()
  target_compile_options(soro-lib PRIVATE
    -Wall
    -Wextra
    -Werror
  )
endif()

file(GLOB_RECURSE soro-test-files test/*.cc)
add_executable(soro-test ${soro-test-files})
target_link_libraries(soro-test doctest soro-lib)
