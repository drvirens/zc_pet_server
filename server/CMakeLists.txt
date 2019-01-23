
# server lib
add_library(_pet_server "")
target_sources(_pet_server
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/lib/src/zc_srv.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/inc/zc_srv.h
        ${CMAKE_CURRENT_LIST_DIR}/lib/inc/zc_socket_path.h
)
target_include_directories(_pet_server
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/lib/inc
)

# server executable
add_executable(pet_server "")
target_sources(pet_server PUBLIC exe/main.c)
target_link_libraries(pet_server _pet_server)
