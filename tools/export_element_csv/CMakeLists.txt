cmake_minimum_required(VERSION 3.19)
project(soro)

file(GLOB_RECURSE export-element-csv-files src/export_element_csv.cc)
add_executable(soro-export-element-csv EXCLUDE_FROM_ALL ${export-element-csv-files})
target_compile_features(soro-export-element-csv PUBLIC ${SORO_COMPILE_FEATURES})
target_link_libraries(soro-export-element-csv pugixml soro-lib)
target_compile_options(soro-export-element-csv PRIVATE ${SORO_CXX_FLAGS})
set_target_properties(soro-export-element-csv PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")