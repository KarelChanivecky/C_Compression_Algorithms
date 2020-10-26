/**
 * @since 2020-10-25
 * @author Karel Chanivecky Garcia
 * BCIT 
 * assignment3
 * compress-huffman.c
 */

#include <dc/unistd.h>
#include <dc/fcntl.h>
#include "fcntl.h"
#include "unistd.h"
#include "huffman_lib.h"

int main(int argc, char ** argv) {
    int src_fd;
    if (argc == 2) {
        src_fd = dc_open(argv[1], O_RDONLY);
    } else {
        src_fd = STDIN_FILENO;
    }
    huffman_compress(src_fd);
    dc_close(src_fd);
}
