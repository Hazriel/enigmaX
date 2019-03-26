cmake_minimum_required(VERSION 3.0)
project(enigmaX)

set(FLAGS_COMMON "-Wall -Wextra -pedantic")
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${FLAGS_COMMON} -O0")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${FLAGS_COMMON} -O3")

set(SOURCES src/options.c src/main.c)

add_executable(enigmaX ${SOURCES})
include_directories(inc)

# TODO: Add library then link it into an executable
