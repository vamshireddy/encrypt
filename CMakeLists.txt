cmake_minimum_required(VERSION 3.13)
project(encrypt C)

set(CMAKE_C_STANDARD 99)

add_executable(encrypt encrypt.c threadpool.c threadpool.h)