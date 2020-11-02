/**
 * @since 2020-10-31
 * @author Karel Chanivecky Garcia
 * BCIT 
 * assignment3
 * lzw_lib.c
 */
//
// Created by karel on 2020-10-31.
//

#include "lzw_lib.h"
#include <string.h>
#include <stdbool.h>
#include <dc/unistd.h>
#include <stdlib.h>
#include "unistd.h"
#include "stdio.h"
#include "stdint.h"
#include "dc/stdlib.h"

#define LZW_TABLE_SIZE 4096
#define BUFFER_SIZE 1
#define BYTES_PER_CYCLE 1
#define INITIAL_TABLE_INSERTION_INDEX 256
#define NOT_FOUND (UINT16_MAX)
#define PADDING 0

uint8_t get_bits( uint16_t container, uint16_t mask, uint8_t shift ) {
    mask <<= shift;
    return ( container & mask ) >> shift;
}

uint8_t get_eight_bits( uint16_t container, uint8_t shift ) {
    return get_bits( container, 255u, shift );
}

uint8_t get_four_bits( uint16_t container, uint8_t shift ) {
    return get_bits( container, 15u, shift );
}

uint16_t get_four_msbs( uint16_t container ) {
    return get_four_bits( container, 8 );
}

uint16_t get_four_lsbs( uint16_t container ) {
    return get_four_bits( container, 0 );
}

uint16_t get_eight_msbs( uint16_t container ) {
    return get_eight_bits( container, 4 );
}

uint16_t get_eight_lsbs( uint16_t container ) {
    return get_eight_bits( container, 0 );
}

uint8_t ** get_initialized_lzw_table() {
    uint8_t ** lzw_table = dc_malloc( sizeof( uint8_t * ) * LZW_TABLE_SIZE );
    for ( uint8_t i = 0; i < UINT8_MAX; i++ ) {
        uint8_t * ch = ( uint8_t * ) dc_malloc( sizeof( uint8_t ) * 2 );
        ch[ 0 ] = i;
        ch[ 1 ] = 0;
        lzw_table[ i ] = ch;
    }
    for ( uint16_t i = UINT8_MAX; i < LZW_TABLE_SIZE; i++ ) {
        uint8_t * empty_ch = ( uint8_t * ) dc_malloc( sizeof( uint8_t ));
        *empty_ch = 0;
        lzw_table[ i ] = empty_ch;
    }
    return lzw_table;
}

void free_table( uint8_t ** lzw_table ) {
    for ( int i = 0; i < LZW_TABLE_SIZE; ++i ) {
        dc_free(( void ** ) &lzw_table[ i ] );
    }
    dc_free(( void ** ) &lzw_table );
}

uint8_t * append( uint8_t * codeword, uint16_t codeword_length, uint8_t ch ) {
    uint8_t * new_codeword = ( uint8_t * ) dc_realloc( codeword, codeword_length + 2 );
    new_codeword[ codeword_length ] = ch;
    new_codeword[ codeword_length + 1 ] = 0;
    return new_codeword;
}

uint16_t index_of_codeword( uint8_t ** lzw_table, uint8_t * codeword, uint16_t codeword_length ) {
    uint16_t index = 0;
    while ( index < LZW_TABLE_SIZE ) {
        if ( strncmp(( char * ) lzw_table[ index ], ( char * ) codeword, codeword_length ) == 0 ) {
            return index;
        }
        index++;
    }
    return NOT_FOUND;
}


void write_codeword( uint16_t index, uint8_t * buffer, bool * half_buffer ) {
    if ( *half_buffer ) {
        *buffer |= get_four_msbs( index );
        dc_write( STDOUT_FILENO, buffer, BUFFER_SIZE );
        *buffer = 0;
        uint8_t remainder = get_eight_lsbs( index );
        dc_write( STDOUT_FILENO, &remainder, BUFFER_SIZE );
    } else {
        uint8_t remainder = get_eight_msbs( index );
        dc_write( STDOUT_FILENO, &remainder, BUFFER_SIZE );
        *buffer |= get_four_lsbs( index ) << 4u;
    }
    *half_buffer = !*half_buffer;
}

void compress_lzw( int src_fd ) {
    uint8_t ** lzw_table = get_initialized_lzw_table();
    uint8_t ch;
    uint16_t codeword_length = 0;
    uint8_t buffer = 0;
    bool half_buffer = false;
    uint16_t table_insertion_index = INITIAL_TABLE_INSERTION_INDEX;
    uint8_t * codeword = ( uint8_t * ) dc_malloc( sizeof( uint8_t ));
    *codeword = 0;
    size_t bytes_read = dc_read( src_fd, &ch, BYTES_PER_CYCLE );
    if ( bytes_read == 0 ) {
        fprintf( stderr, "File is empty!" );
        dc_close( src_fd );
        exit( EXIT_FAILURE );
    }

    while ( 0 < bytes_read ) {
        uint16_t possible_code = index_of_codeword( lzw_table, codeword, codeword_length );
        codeword = append( codeword, codeword_length, ch );
        codeword_length++;
        if ( index_of_codeword( lzw_table, codeword, codeword_length ) == NOT_FOUND) {
            lzw_table[ table_insertion_index ] = codeword;
            write_codeword( possible_code, &buffer, &half_buffer );
            table_insertion_index++;
            if ( table_insertion_index == LZW_TABLE_SIZE ) {
                table_insertion_index = INITIAL_TABLE_INSERTION_INDEX;
            }
            codeword = ( uint8_t * ) dc_malloc( sizeof( uint8_t ) * 2 );
            codeword[ 0 ] = ch;
            codeword[ 1 ] = 0;
            codeword_length = 1;
        }
        bytes_read = dc_read( src_fd, &ch, BYTES_PER_CYCLE );
    }
    uint16_t index = index_of_codeword( lzw_table, codeword, codeword_length );
    write_codeword( index, &buffer, &half_buffer );
    if (half_buffer) {
        write_codeword( PADDING, &buffer, &half_buffer );
    }
    free_table( lzw_table );
}

uint8_t get_ch(uint16_t first, uint16_t second){

}

uint16_t extract_char(uint8_t * ch, uint16_t code, bool half_buffer){
    uint16_t remainder;
    if(!half_buffer){
        remainder = get_four_lsbs(code);
        uint8_t first = get_four_bits(code,);
        uint8_t second = get_four_bits(code,);
        ch = get_ch(first, second);
    }

}

void decompress_lzw(int src_fd){
    uint8_t ** lzw_table = get_initialized_lzw_table();
    uint16_t table_insertion_index = INITIAL_TABLE_INSERTION_INDEX;
    uint16_t code;

    bool half_buffer = false;
    uint8_t ch = 0;
    uint16_t remainder;

    size_t bytes_read = dc_read( src_fd, &code, (BYTES_PER_CYCLE * 2) );
    if (bytes_read == 0) {
        fprintf(stderr, "File is empty!");
        dc_close(src_fd);
        exit(EXIT_FAILURE);
    }



}

//    uint8_t first_in;
//    uint8_t second_in;
//    uint8_t third_in;

//size_t bytes_read = dc_read( src_fd, &first_in, BYTES_PER_CYCLE );
//if ( bytes_read == 0 ) {
//fprintf( stderr, "File is empty!" );
//dc_close( src_fd );
//exit( EXIT_FAILURE );
//}
//
//bytes_read = dc_read( src_fd, &second_in, BYTES_PER_CYCLE );
//if ( bytes_read == 0 ) {
//fprintf( stderr, "File is corrupt!" );
//dc_close( src_fd );
//exit( EXIT_FAILURE );
//}
//
//bytes_read = dc_read( src_fd, &third_in, BYTES_PER_CYCLE );
//if ( bytes_read == 0 ) {
//half_buffer = true;
//}
//
//remainder = extract_char(&ch, first_in, second_in, third_in, half_buffer);

#pragma clang diagnostic pop
