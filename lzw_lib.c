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

uint16_t get_four_msbs_16b( uint16_t container ) {
    return get_four_bits( container, 8 );
}

uint16_t get_four_msbs_8b( uint8_t container ) {
    return get_four_bits( container, 4 );
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
    uint8_t * new_codeword = ( uint8_t * ) dc_malloc( sizeof( uint8_t ) * ( codeword_length + 2 ));
    strncpy(( char * ) new_codeword, ( char * ) codeword, codeword_length );
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
        *buffer |= get_four_msbs_16b( index );
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
        free_table( lzw_table );
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
    if ( half_buffer ) {
        dc_write(STDOUT_FILENO, &buffer, BUFFER_SIZE);
    }
    free_table( lzw_table );
}


uint8_t read_codeword( int src_fd, uint16_t * ch, uint8_t * buffer, bool * half_buffer ) {
    uint8_t bytes_read;
    *ch = 0;
    if ( *half_buffer ) {
        *ch = get_four_lsbs( *buffer ) << 8u;
        bytes_read = dc_read( src_fd, buffer, BUFFER_SIZE );
        *ch |= *buffer;
        *buffer = 0;
    } else {
        bytes_read = dc_read( src_fd, ch, BUFFER_SIZE );
        *ch <<= 4u;
        bytes_read += dc_read( src_fd, buffer, BUFFER_SIZE );
        *ch |= get_four_msbs_8b( *buffer );
    }
    *half_buffer = !*half_buffer;
    return bytes_read;
}

void decompress_lzw( int src_fd ) {
    uint8_t ** lzw_table = get_initialized_lzw_table();
    uint16_t table_insertion_index = INITIAL_TABLE_INSERTION_INDEX;
    uint16_t max_table_index = 255;
    uint16_t codeword_index = 0;
    uint16_t codeword_length;
    uint8_t buffer = 0;
    uint8_t * codeword = ( uint8_t * ) dc_malloc( sizeof( uint8_t ));
    *codeword = 0;
    bool half_buffer = false;

    size_t bytes_read = read_codeword( src_fd, &codeword_index, &buffer, &half_buffer );
    if ( bytes_read != 2 ) {
        fprintf( stderr, "Incorrect file size!" );
        dc_close( src_fd );
        free_table( lzw_table );
        exit( EXIT_FAILURE );
    }
    write( STDOUT_FILENO, lzw_table[ codeword_index ], BYTES_PER_CYCLE );

    while ( bytes_read ) {
        codeword = lzw_table[ codeword_index ];
        codeword_length = strlen(( char * ) codeword );
        bytes_read = read_codeword( src_fd, &codeword_index, &buffer, &half_buffer );
        if ( !bytes_read ) {
            continue;
        }
        if ( codeword_index <= max_table_index ) {
            codeword = append( codeword, codeword_length, *lzw_table[ codeword_index ] );
            lzw_table[ table_insertion_index ] = codeword;
            dc_write( STDOUT_FILENO, lzw_table[ codeword_index ], strlen(( char * ) lzw_table[ codeword_index ] ));
        } else {
            codeword = append( codeword, codeword_length, *codeword );
            codeword_length++;
            lzw_table[ table_insertion_index ] = codeword;
            dc_write( STDOUT_FILENO, codeword, codeword_length );
        }
        max_table_index = max_table_index == LZW_TABLE_SIZE ?
                          LZW_TABLE_SIZE : max_table_index + 1;
        table_insertion_index = table_insertion_index == LZW_TABLE_SIZE ?
                                INITIAL_TABLE_INSERTION_INDEX : table_insertion_index + 1;
    }
    free_table(lzw_table);
}
