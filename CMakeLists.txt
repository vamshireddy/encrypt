cmake_minimum_required(VERSION 3.13)
project(encrypt C)

set(CMAKE_C_STANDARD 99)

add_executable(encrypt src/encrypt.c src/sharedbuffer.c inc/sharedbuffer.h src/mypool.c inc/mypool.h tests/main.c src/wrappers.c inc/wrappers.h)