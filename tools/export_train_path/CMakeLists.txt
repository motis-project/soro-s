cmake_minimum_required(VERSION 3.19)
project(soro)

file(GLOB_RECURSE export-train-path-files src/export_train_path.cc)
add_executable(soro-export-train-path EXCLUDE_FROM_ALL ${export-train-path-files})
target_compile_features(soro-export-train-path PUBLIC cxx_std_23)
target_link_libraries(soro-export-train-path pugixml soro-lib)
target_compile_options(soro-export-train-path PRIVATE ${SORO_CXX_FLAGS})
set_target_properties(soro-export-train-path PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")