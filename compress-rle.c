//
// Created by animesh on 2020-10-26.
//

#include <dc/unistd.h>
#include <dc/fcntl.h>
#include "fcntl.h"
#include "unistd.h"
#include "rle_lib.h"

int main(int argc, char ** argv) {
    int src_fd;
    if (argc == 2) {
        src_fd = open(argv[1], O_RDONLY);
    } else {
        src_fd = STDIN_FILENO;
    }
    compress_rle(src_fd);
    if(argc == 2){
        close(src_fd);
    }

}