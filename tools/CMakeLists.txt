cmake_minimum_required(VERSION 3.19)
project(soro)

set(all-tools "export_exclusion_graph")

foreach(tool ${all-tools})
  add_subdirectory(${tool})
endforeach(tool)