cmake_minimum_required(VERSION 3.5)

project(prog1 LANGUAGES C)

add_executable(prog1 main.c)

install(TARGETS prog1
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

target_link_libraries(prog1 -lrt -pthread)
