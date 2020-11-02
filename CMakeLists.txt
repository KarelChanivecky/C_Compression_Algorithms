cmake_minimum_required(VERSION 3.17)
project(assignment3 C)

set(CMAKE_C_STANDARD 11)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

add_library(huffman_lib SHARED huffman_lib.c)
target_link_libraries(huffman_lib dc dc_utils rt)
target_compile_options(huffman_lib PRIVATE -Wpedantic -Wall -Wextra -fPIC)

add_executable(compress-huffman compress-huffman.c)
target_link_libraries(compress-huffman huffman_lib)
target_compile_options(compress-huffman PRIVATE -Wpedantic -Wall -Wextra -fPIC)

add_executable(decompress-huffman decompress-huffman.c)
target_link_libraries(decompress-huffman huffman_lib)
target_compile_options(decompress-huffman PRIVATE -Wpedantic -Wall -Wextra -fPIC)

add_library(rle_lib SHARED rle_lib.c)
target_compile_options(rle_lib PRIVATE -Wpedantic -Wall -Wextra -fPIC)

add_executable(compress-rle compress-rle.c )
target_link_libraries(compress-rle rle_lib)
target_compile_options(compress-rle PRIVATE -Wpedantic -Wall -Wextra -fPIC)

add_executable(decompress-rle decompress-rle.c )
target_link_libraries(decompress-rle rle_lib)
target_compile_options(decompress-rle PRIVATE -Wpedantic -Wall -Wextra -fPIC)

add_library(lzw_lib SHARED lzw_lib.c)
target_link_libraries(lzw_lib dc rt)
target_compile_options(lzw_lib PRIVATE -Wpedantic -Wall -Wextra -fPIC)

add_executable(compress-lzw compress-lzw.c )
target_link_libraries(compress-lzw lzw_lib)
target_compile_options(compress-lzw PRIVATE -Wpedantic -Wall -Wextra -fPIC)

add_executable(decompress-lzw decompress-lzw.c )
target_link_libraries(decompress-lzw lzw_lib)
target_compile_options(decompress-lzw PRIVATE -Wpedantic -Wall -Wextra -fPIC)
