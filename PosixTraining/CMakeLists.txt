cmake_minimum_required(VERSION 3.5)

project(PosixTraining LANGUAGES C)

add_executable(PosixTraining main.c simpleshell.c simpleshell.h)

install(TARGETS PosixTraining
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

target_link_libraries(PosixTraining -lrt -pthread)
