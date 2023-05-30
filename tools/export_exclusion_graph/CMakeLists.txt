cmake_minimum_required(VERSION 3.19)
project(soro)

file(GLOB_RECURSE export-exclusion-graph-files src/export_exclusion_graph.cc)
add_executable(soro-export-exclusion-graph EXCLUDE_FROM_ALL ${export-exclusion-graph-files})
target_compile_features(soro-export-exclusion-graph PUBLIC cxx_std_23)
target_link_libraries(soro-export-exclusion-graph pugixml soro-lib)
target_compile_options(soro-export-exclusion-graph PRIVATE ${SORO_CXX_FLAGS})
set_target_properties(soro-export-exclusion-graph PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")