cmake_minimum_required(VERSION 3.19)
project(soro)

set(all-tools "export_exclusion_graph" "export_element_csv")

foreach (tool ${all-tools})
    add_subdirectory(${tool})
endforeach (tool)