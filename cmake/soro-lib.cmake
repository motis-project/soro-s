# This file is responsible for defining the 'soro-lib' target

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

file(GLOB_RECURSE soro-lib-files src/*.cc)
add_library(soro-lib STATIC EXCLUDE_FROM_ALL ${soro-lib-files})

if (SORO_CUDA)
    add_library(infrastructure-cuda src/infrastructure/gpu/exclusion.cu)
    set_target_properties(infrastructure-cuda PROPERTIES
            WINDOWS_EXPORT_ALL_SYMBOLS ON
            CUDA_STANDARD 20
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
            INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/")
    target_include_directories(infrastructure-cuda PUBLIC include)
    set_property(TARGET infrastructure-cuda PROPERTY CUDA_ARCHITECTURES 75 61)
endif ()

if (${CMAKE_USE_PTHREADS_INIT})
    set(SORO_LIB_COMPILE_OPTIONS "-pthread")
endif ()

target_compile_options(soro-lib PRIVATE ${SORO_COMPILE_OPTIONS} ${SORO_LIB_COMPILE_OPTIONS})
target_compile_features(soro-lib PRIVATE ${SORO_COMPILE_FEATURES})
target_compile_definitions(soro-lib PUBLIC ${SORO_COMPILE_DEFINITIONS})
target_include_directories(soro-lib PUBLIC include)
target_link_libraries(soro-lib PUBLIC utl cista date pugixml tar Threads::Threads)

if (SORO_CUDA)
    target_link_libraries(soro-lib PUBLIC infrastructure-cuda)
endif ()
