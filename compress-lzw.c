//
// Created by animesh on 2020-10-31.
//


#include "fcntl.h"
#include "unistd.h"
#include "lzw_lib.h"

int main(int argc, char * argv[]) {
    int src_fd;
    if (argc == 2) {
        src_fd = open(argv[1], O_RDONLY);
    } else {
        src_fd = STDIN_FILENO;
    }
    compress_lzw(src_fd);
    if(argc == 2){
        close(src_fd);
    }

}
