add_subdirectory(server)
add_subdirectory(client)

add_custom_target(soro-server-client)
add_dependencies(soro-server-client soro-server)
add_dependencies(soro-server-client soro-client)