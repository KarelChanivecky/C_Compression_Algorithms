//
// Created by animesh on 2020-10-26.
//

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>

#include "rle_lib.h"

#define BYTES_PER_CYCLE 1u
#define GENERIC_ERROR -1

size_t h_read( int fd, void * buf, size_t b_to_read ) {
    ssize_t b_read = read( fd, buf, b_to_read );
    if ( b_read == GENERIC_ERROR ) {
        fprintf( stderr, "%s", strerror(errno));
        exit( EXIT_FAILURE );
    }
    return b_read;
}

size_t h_write( int fd, void * buf, size_t b_to_write ) {
    ssize_t b_written = write( fd, buf, b_to_write );
    if ( b_written == GENERIC_ERROR ) {
        fprintf( stderr, "%s", strerror(errno));
        exit( EXIT_FAILURE );
    }
    return b_written;
}

void decompress_rle( int src_fd ) {
    uint8_t in;
    size_t c_read;
    uint8_t sen;
    c_read = h_read( src_fd, &in, BYTES_PER_CYCLE );
    if (c_read == 0) {
        fprintf(stderr, "File is empty!");
        exit(EXIT_FAILURE);
    }
    sen = in;
    c_read = h_read( src_fd, &in, BYTES_PER_CYCLE );
    while ( c_read > 0 ) {
        if ( in != sen ) {
            h_write( STDOUT_FILENO, &in, BYTES_PER_CYCLE );
        } else if ( in == 37 ) {
            uint8_t count;
            h_read( src_fd, &count, BYTES_PER_CYCLE );
            uint8_t c;
            h_read( src_fd, &c, BYTES_PER_CYCLE );
            for ( uint8_t i = 0; i < count; i++ )
                h_write( STDOUT_FILENO, &c, BYTES_PER_CYCLE );
        }
        c_read = h_read( src_fd, &in, BYTES_PER_CYCLE );
    }
}

void compress_rle( int src_fd ) {
    uint8_t in;
    uint8_t max = UINT8_MAX;
    unsigned int counter = 0;
    unsigned int sen_counter = 0;
    uint8_t temp = 0;
    uint8_t sen = 37;
    h_write( STDOUT_FILENO, &sen, BYTES_PER_CYCLE ); // print sentinel as first char.

    size_t c_read;
    c_read = h_read( src_fd, &temp, BYTES_PER_CYCLE );
    if ( c_read > 0 ) {
        if ( temp == 37 ) {
            sen_counter++;
        } else {
            counter++;
        }
    } else {
        fprintf( stdout, "%s", "No input provided" );
        exit( EXIT_SUCCESS );
    }

    c_read = h_read( src_fd, &in, BYTES_PER_CYCLE );

    while ( c_read > 0 ) {
        if ( in == temp ) {
            if ( in == 37 ) {
                sen_counter++;
                temp = in;
            } else {
                counter++;
                temp = in;
            }
        } else {
            if ( temp == 37 ) {
                if ( sen_counter <= 255 ) {
                    h_write( STDOUT_FILENO, &sen, BYTES_PER_CYCLE );
                    h_write( STDOUT_FILENO, &sen_counter, BYTES_PER_CYCLE );
                    h_write( STDOUT_FILENO, &sen, BYTES_PER_CYCLE );
                    sen_counter = 0;
                    counter = 1;
                } else {
                    while ( sen_counter > 255 ) {
                        h_write( STDOUT_FILENO, &sen, BYTES_PER_CYCLE );
                        h_write( STDOUT_FILENO, &max, BYTES_PER_CYCLE );
                        h_write( STDOUT_FILENO, &sen, BYTES_PER_CYCLE );
                        sen_counter = sen_counter - 255;
                    }
                    h_write( STDOUT_FILENO, &sen, BYTES_PER_CYCLE );
                    h_write( STDOUT_FILENO, &sen_counter, BYTES_PER_CYCLE );
                    h_write( STDOUT_FILENO, &sen, BYTES_PER_CYCLE );
                    sen_counter = 0;
                    counter = 1;
                }
                temp = in;
            } else {
                if ( counter <= 3 ) {
                    for ( unsigned int i = 0; i < counter; i++ )
                        h_write( STDOUT_FILENO, &temp, BYTES_PER_CYCLE );
                    counter = 0;
                    if ( in == 37 ) {
                        sen_counter++;
                    } else {
                        counter++;
                    }
                } else if ( counter > 3 && counter <= 255 ) {
                    h_write( STDOUT_FILENO, &sen, BYTES_PER_CYCLE );
                    h_write( STDOUT_FILENO, &counter, BYTES_PER_CYCLE );
                    h_write( STDOUT_FILENO, &temp, BYTES_PER_CYCLE );
                    counter = 0;
                    if ( in == 37 ) {
                        sen_counter++;
                    } else {
                        counter++;
                    }
                } else if ( counter > 255 ) {
                    h_write( STDOUT_FILENO, &sen, BYTES_PER_CYCLE );
                    h_write( STDOUT_FILENO, &max, BYTES_PER_CYCLE );
                    h_write( STDOUT_FILENO, &temp, BYTES_PER_CYCLE );
                    counter = counter - 255;
                    h_write( STDOUT_FILENO, &sen, BYTES_PER_CYCLE );
                    h_write( STDOUT_FILENO, &counter, BYTES_PER_CYCLE );
                    h_write( STDOUT_FILENO, &temp, BYTES_PER_CYCLE );
                    counter = 0;
                    if ( in == 37 ) {
                        sen_counter++;
                    } else {
                        counter++;
                    }
                }
                temp = in;
            }
        }
        c_read = h_read( src_fd, &in, BYTES_PER_CYCLE );
    }

    if ( sen_counter > 0 ) {
        if ( sen_counter <= 255 ) {
            h_write( STDOUT_FILENO, &sen, BYTES_PER_CYCLE );
            h_write( STDOUT_FILENO, &sen_counter, BYTES_PER_CYCLE );
            h_write( STDOUT_FILENO, &sen, BYTES_PER_CYCLE );
            sen_counter = 0;
            counter = 1;
        } else {
            while ( sen_counter > 255 ) {
                h_write( STDOUT_FILENO, &sen, BYTES_PER_CYCLE );
                h_write( STDOUT_FILENO, &max, BYTES_PER_CYCLE );
                h_write( STDOUT_FILENO, &sen, BYTES_PER_CYCLE );
                sen_counter = sen_counter - 255;
            }
            h_write( STDOUT_FILENO, &sen, BYTES_PER_CYCLE );
            h_write( STDOUT_FILENO, &sen_counter, BYTES_PER_CYCLE );
            h_write( STDOUT_FILENO, &sen, BYTES_PER_CYCLE );
            sen_counter = 0;
            counter = 1;
        }
    } else if ( counter > 0 ) {
        if ( counter <= 3 ) {
            for ( unsigned int i = 0; i < counter; i++ )
                h_write( STDOUT_FILENO, &temp, BYTES_PER_CYCLE );
            counter = 0;
        } else if ( counter > 3 && counter <= 255 ) {
            h_write( STDOUT_FILENO, &sen, BYTES_PER_CYCLE );
            h_write( STDOUT_FILENO, &counter, BYTES_PER_CYCLE );
            h_write( STDOUT_FILENO, &temp, BYTES_PER_CYCLE );
        } else if ( counter > 255 ) {
            while ( counter > 255 ) {
                h_write( STDOUT_FILENO, &sen, BYTES_PER_CYCLE );
                h_write( STDOUT_FILENO, &max, BYTES_PER_CYCLE );
                h_write( STDOUT_FILENO, &temp, BYTES_PER_CYCLE );
                counter = counter - 255;
            }
            h_write( STDOUT_FILENO, &sen, BYTES_PER_CYCLE );
            h_write( STDOUT_FILENO, &counter, BYTES_PER_CYCLE );
            h_write( STDOUT_FILENO, &temp, BYTES_PER_CYCLE );
        }
    }

}

