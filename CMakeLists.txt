cmake_minimum_required(VERSION 3.17)
project(assignment3 C)

set(CMAKE_C_STANDARD 11)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

add_library(huffman_lib huffman_lib.c huffman_lib.h)
target_link_libraries(huffman_lib dc dc_utils rt)
target_compile_options(huffman_lib PRIVATE -Wpedantic -Wall -Wextra -fPIC)

add_executable(compress-huffman huffman_lib.c huffman_lib.h compress-huffman.c)
target_link_libraries(compress-huffman dc dc_utils rt)
target_compile_options(compress-huffman PRIVATE -Wpedantic -Wall -Wextra -fPIC)

add_executable(decompress-huffman huffman_lib.c huffman_lib.h decompress-huffman.c)
target_link_libraries(decompress-huffman dc dc_utils rt)
target_compile_options(decompress-huffman PRIVATE -Wpedantic -Wall -Wextra -fPIC)

add_library(rle_lib rle_lib.c rle_lib.h)
target_compile_options(rle_lib PRIVATE -Wpedantic -Wall -Wextra -fPIC)

add_executable(compress-rle rle_lib.c rle_lib.h compress-rle.c )
target_compile_options(compress-rle PRIVATE -Wpedantic -Wall -Wextra -fPIC)

add_executable(decompress-rle rle_lib.c rle_lib.h decompress-rle.c )
target_compile_options(decompress-rle PRIVATE -Wpedantic -Wall -Wextra -fPIC)

add_library(lzw_lib lzw_lib.c lzw_lib.h)
target_compile_options(lzw_lib PRIVATE -Wpedantic -Wall -Wextra -fPIC)
#
#add_executable(compress-rle rle_lib.c rle_lib.h compress-rle.c )
#target_compile_options(compress-rle PRIVATE -Wpedantic -Wall -Wextra -fPIC)
#
#add_executable(decompress-rle rle_lib.c rle_lib.h decompress-rle.c )
#target_compile_options(decompress-rle PRIVATE -Wpedantic -Wall -Wextra -fPIC)