cmake_minimum_required(VERSION 3.19)
project(soro)

set(all-tools "export_exclusion_graph" "export_train_path" "find_duplicate_trains")

foreach(tool ${all-tools})
  add_subdirectory(${tool})
endforeach(tool)