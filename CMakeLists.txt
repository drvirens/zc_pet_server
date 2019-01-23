cmake_minimum_required(VERSION 3.9)
project(prj_pet_server
    VERSION 0.0.1
    DESCRIPTION "Pet server project"
    LANGUAGES C
)

add_subdirectory(client)
add_subdirectory(server)