add_executable(pet_client "")
target_sources(pet_client
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${CMAKE_CURRENT_LIST_DIR}/zc_socket_path.h
)