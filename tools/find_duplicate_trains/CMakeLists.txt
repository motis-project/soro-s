cmake_minimum_required(VERSION 3.19)
project(soro)

file(GLOB_RECURSE find-duplicate-trains-files src/find_duplicate_trains.cc)
add_executable(find-duplicate-trains EXCLUDE_FROM_ALL ${find-duplicate-trains-files})
target_compile_features(find-duplicate-trains PUBLIC cxx_std_23)
target_link_libraries(find-duplicate-trains pugixml soro-lib)
target_compile_options(find-duplicate-trains PRIVATE ${SORO_CXX_FLAGS})
set_target_properties(find-duplicate-trains PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")