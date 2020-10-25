cmake_minimum_required(VERSION 3.17)
project(assignment3 C)

set(CMAKE_C_STANDARD 11)

add_executable(assignment3 main.c huffman_lib.c huffman_lib.h)

target_compile_options(assignment3 PRIVATE -Wpedantic -Wall -Wextra -fPIC)
