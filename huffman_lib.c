/**
 * 10/21/2020
 * Created by Karel Chanivecky Garcia.
 * A01052674
 *
 * BCIT CST
 * Set O Datacomm
 *
 * assignment3
 */

#include "huffman_lib.h"
#include <dc_utils/dlinked_list.h>
#include <dc_utils/bit_array.h>
#include <dc_utils/dc_tree.h>
#include <dc/stdlib.h>
#include <dc/unistd.h>
#include <stdlib.h>
#include "stdbool.h"
#include "limits.h"
#include "stdio.h"
#include "arpa/inet.h"

#define TEMP_FILENAME "huffman_tempXXXXXXX"
#define BYTES_PER_CHAR 1
#define BYTES_PER_CHAR_COUNT 4
#define BUFFER_SIZE 1
#define LEFT_BRANCH 0
#define RIGHT_BRANCH 1
#define END_OF_PATH 2

struct char_container_s {
    uint8_t ch;
    uint32_t count;
};
typedef struct char_container_s char_container;

void free_element( void * element, void * _ ) {
    (void)_;
    if ( !element ) {
        return;
    }
    dc_free( &element );
}

void free_tree_contents( void * v_tree ) {
    if ( !v_tree ) {
        return;
    }
    dc_tree_node ** tree = v_tree;
    dc_tree_map( *tree, free_element, NULL);
    dc_tree_free_branch( tree );
}
char_container * create_char_container( uint8_t ch ) {
    char_container * new_char_data = ( char_container * ) dc_malloc( sizeof( struct char_container_s ));
    new_char_data->ch = ch;
    new_char_data->count = 1;
    return new_char_data;
}

char_container * create_char_container_with_count( uint8_t ch, size_t count ) {
    char_container * char_wrapper = create_char_container( ch );
    char_wrapper->count = count;
    return char_wrapper;
}


size_t get_count_from_tree_node( void * data ) {
    dc_tree_node * tree_node = ( dc_tree_node * ) data;
    char_container * ch_data = ( char_container * ) tree_node->content;
    return ch_data->count;
}

uint8_t get_char_from_tree_node( void * v_ch ) {
    dc_tree_node * t_ch = ( dc_tree_node * ) v_ch;
    char_container * c_ch = ( char_container * ) t_ch->content;
    return c_ch->ch;
}

dc_tree_node * create_branch_node( void * lhs, void * rhs ) {
    char_container * wrapper = create_char_container( 0 );
    wrapper->count = get_count_from_tree_node( lhs ) + get_count_from_tree_node( rhs );
    dc_tree_node * root = dc_tree_create_node( wrapper );
    root->left = ( dc_tree_node * ) lhs;
    root->right = rhs;
    return root;
}

int has_value_comparator( void * v_lhs, void * v_rhs ) {
    uint8_t lhs = get_char_from_tree_node( v_lhs );
    uint8_t rhs = get_char_from_tree_node( v_rhs );
    return lhs == rhs;
}

void add_char( dlinked_list * char_counts, uint8_t ch ) {
    char_container * ch_wrapper = create_char_container( ch );
    size_t index;
    if ( dlinked_index_of_value( char_counts, dc_tree_create_node( ch_wrapper ), has_value_comparator, &index )
         == SUCCESS ) {
        dc_tree_node * t_ch = dlinked_get_value( char_counts, index );
        char_container * tgt_ch = ( char_container * ) t_ch->content;
        tgt_ch->count++;
    } else {
        dlinked_push( char_counts, dc_tree_create_node( ch_wrapper ));
    }
}

dlinked_list * get_char_counts( int src_fd, int temp_fd ) {
    dlinked_list * char_counts = dlinked_create_list();
    uint8_t ch;
    size_t bytes_read = 0;
    while ( dc_read( src_fd, &ch, BYTES_PER_CHAR )) {
        bytes_read++;
        dc_write( temp_fd, &ch, BYTES_PER_CHAR );
        add_char( char_counts, ch );
    }
    if ( bytes_read == 0 ) {
        fprintf( stderr, "File is empty!" );
        exit( EXIT_FAILURE );
    }
    if ( bytes_read == 1 ) {
        fprintf( stderr, "Cannot compress a file with only one byte!" );
        exit( EXIT_FAILURE );
    }
    return char_counts;
}

int compare_uint8_t( uint8_t lhs, uint8_t rhs ) {
    if ( lhs < rhs ) {
        return LHS_SMALLER;
    } else if ( lhs == rhs ) {
        return LHS_EQUAL;
    }
    return LHS_LARGER;
}

int sorting_comparator( void * v_lhs, void * v_rhs ) {
    size_t c_lhs = get_count_from_tree_node( v_lhs );
    size_t c_rhs = get_count_from_tree_node( v_rhs );
    if ( c_lhs < c_rhs ) {
        return LHS_SMALLER;
    }
    if ( c_lhs == c_rhs ) {
        uint8_t ch_lhs = get_char_from_tree_node( v_lhs );
        uint8_t ch_rhs = get_char_from_tree_node( v_rhs );
        return compare_uint8_t( ch_lhs, ch_rhs );
    }
    return LHS_LARGER;
}

int insertion_comparator( void * v_el, void * v_key ) {
    size_t c_el = get_count_from_tree_node( v_el );
    size_t c_key = get_count_from_tree_node( v_key );
    if ( c_el < c_key ) {
        return false;
    }
    return true;
}

dc_tree_node * make_huffman_tree( dlinked_list * ch_counts ) {
    if ( ch_counts->size == 0 ) {
        return NULL;
    }
    if ( ch_counts->size == 1 ) {
        dc_tree_node * symbol_node = ( dc_tree_node * ) dlinked_pop_head( ch_counts );
        char_container * root_ch_container =
                create_char_container_with_count( 0, (( char_container * ) symbol_node->content )->count );
        dc_tree_node * root = dc_tree_create_node( root_ch_container );
        root->right = symbol_node;
        return root;
    }
    while ( 1 < ch_counts->size ) {
        void * l_branch = dlinked_pop_head( ch_counts );
        void * r_branch = dlinked_pop_head( ch_counts );
        dc_tree_node * root = create_branch_node( l_branch, r_branch );
        size_t insertion_index = 0;
        int search_status = dlinked_index_of_value( ch_counts, root, insertion_comparator, &insertion_index );
        if ( search_status == FAILURE || ( search_status != BAD_ARGS && insertion_index == ch_counts->size )) {
            dlinked_push( ch_counts, root );
        } else {
            dlinked_insert_value( ch_counts, insertion_index, root );
        }
    }
    return dlinked_pop_head( ch_counts );
}

void write_char( void * v_tree_node ) {
    dc_tree_node * tree = ( dc_tree_node * ) v_tree_node;
    char_container * char_wrapper = ( char_container * ) tree->content;
    uint8_t ch = char_wrapper->ch;
    uint32_t count = char_wrapper->count;
    uint32_t net_count = htonl(count);
    write( STDOUT_FILENO, &ch, BYTES_PER_CHAR );
    write( STDOUT_FILENO, &net_count, BYTES_PER_CHAR_COUNT );
}

void write_char_counts( dlinked_list * char_counts ) {
    dlink_map( char_counts, write_char );
    uint8_t ch = get_char_from_tree_node( char_counts->head->content );
    write( STDOUT_FILENO, &ch, BYTES_PER_CHAR );
}

bool get_path_to_char( dc_tree_node * tree, uint8_t ch, uint8_t * path, size_t current_depth ) {
    if ( !tree ) {
        return false;
    }
    char_container * char_wrapper = ( char_container * ) tree->content;
    if ( char_wrapper->ch == ch ) {
        path[ current_depth ] = END_OF_PATH;
        return true;
    }
    bool in_left_branch = get_path_to_char( tree->left, ch, path, current_depth + 1 );
    bool in_right_branch = get_path_to_char( tree->right, ch, path, current_depth + 1 );
    if ( in_left_branch ) {
        path[ current_depth ] = LEFT_BRANCH;
        return true;
    }
    if ( in_right_branch ) {
        path[ current_depth ] = RIGHT_BRANCH;
        return true;
    }
    return false;
}

void encode_file( int src_fd, dc_tree_node * huffman_tree ) {
    uint8_t buffer = 0;
    uint8_t buffer_index = 0; // keep track of how many bits have been pushed to buffer

    uint8_t ch = 0;
    bool path_done;
    size_t tree_height = dc_tree_height( huffman_tree ); // maximum path length
    uint8_t path[tree_height + 1]; // extra char for termination

    size_t bytes_read = dc_read( src_fd, &ch, BYTES_PER_CHAR );
    while ( 0 < bytes_read ) { // while we haven't read the whole file
        get_path_to_char( huffman_tree, ch, path, 0 );
        size_t path_index = 0;
        path_done = false;
        while ( !path_done ) {
            // while we have not completely processed the current path
            // write path
            while ( path[ path_index ] != END_OF_PATH && buffer_index < CHAR_BIT ) {
                // while buffer is not full and we have not reached the end of the path
                buffer <<= 1u;
                buffer |= path[ path_index ];
                path_index++;
                buffer_index++;
            }

            // if the path is over, step out of loop
            if ( path[ path_index ] == END_OF_PATH ) {
                path_done = true;
            }

            // if buffer is full, then write
            if ( buffer_index == CHAR_BIT ) {
                write( STDOUT_FILENO, &buffer, 1 );
                buffer = 0;
                buffer_index = 0;
            }
        }
        bytes_read = dc_read( src_fd, &ch, BYTES_PER_CHAR );
    }
    // there may be some bits left in the buffer
    if ( buffer_index != 0 ) {
        buffer <<= ( uint8_t ) ( CHAR_BIT - buffer_index );
        write( STDOUT_FILENO, &buffer, 1 );
    }
}


void huffman_compress( int src_fd ) {
    char temp_filename[] = TEMP_FILENAME;
    int temp_fd = dc_mkstemp( temp_filename );
    off_t file_start = dc_lseek( temp_fd, 0, SEEK_CUR );
    dlinked_list * char_list = get_char_counts( src_fd, temp_fd );
    write_char_counts( char_list );
    dlinked_list * sorted_list = dlinked_quicksort( char_list, sorting_comparator );
    dc_lseek( temp_fd, file_start, SEEK_SET );
    dc_tree_node * huffman_tree = make_huffman_tree( sorted_list );
    encode_file( temp_fd, huffman_tree );
    dlinked_free_list( &char_list );
    dc_tree_free_branch( &huffman_tree );
    dc_close( temp_fd );
    dc_unlink( temp_filename );
}

dlinked_list * parse_huffman_header( int src_fd ) {
    dlinked_list * char_counts = dlinked_create_list();
    uint8_t header_terminator;
    uint8_t ch;
    uint32_t ch_count;
    uint32_t net_ch_count;
    size_t bytes_read = read( src_fd, &ch, BYTES_PER_CHAR );
    header_terminator = ch;
    bool done = false;
    while ( !done && bytes_read ) {
        bytes_read = read( src_fd, &net_ch_count, BYTES_PER_CHAR_COUNT );
        if ( bytes_read < BYTES_PER_CHAR_COUNT ) {
            fprintf( stderr, "ERROR! Incorrect file size!" );
            exit( EXIT_FAILURE );
        }
        ch_count = ntohl(net_ch_count);
        dlinked_push_head( char_counts, dc_tree_create_node( create_char_container_with_count( ch, ch_count )));
        bytes_read = read( src_fd, &ch, BYTES_PER_CHAR );
        if ( ch == header_terminator ) {
            done = true;
        }
    }
    if ( !done ) {
        fprintf( stderr, "ERROR! Incorrect file size!" );
        exit( EXIT_FAILURE );
    }
    return char_counts;
}

uint8_t tree_bits(dc_tree_node * tree, size_t height) {
    if (dc_tree_is_leaf(tree)) {
        char_container * char_wrapper = (char_container*) tree->content;
        return height * char_wrapper->count;
    }
    return tree_bits(tree->left, height + 1) +
    tree_bits(tree->right, height + 1);
}

uint8_t get_spillover_bit_count( dc_tree_node * huffman_tree ) {
    uint8_t last_bits = tree_bits(huffman_tree, 0) % 8;
    return last_bits == 0 ? 8 : last_bits;
}

dc_tree_node * decompress_char( dc_tree_node * huffman_tree,
                                dc_tree_node * current_node,
                                bit_array * bit_arr,
                                uint8_t relevant_bits_count ) {
    while ( 0 < relevant_bits_count-- ) {
        bool branch = bit_array_check_bit( bit_arr, bit_arr->bit_length - 1 );
        bit_array_pop_bit( bit_arr );
        if ( branch == RIGHT_BRANCH ) {
            current_node = current_node->right;
        } else {
            current_node = current_node->left;
        }
        if ( dc_tree_is_leaf( current_node )) {
            char_container * char_wrapper = ( char_container * ) current_node->content;
            write( STDOUT_FILENO, &char_wrapper->ch, BYTES_PER_CHAR );
            current_node = huffman_tree;
        }
    }
    return current_node;
}

bit_array * get_bit_array( uint8_t content ) {
    bit_array * bit_arr = bit_array_create();
    bit_array_init( bit_arr, CHAR_BIT );
    bit_array_add_byte( bit_arr, content );
    return bit_arr;
}

void decode_file( int src_fd, dc_tree_node * huffman_tree, uint8_t spill_over_bit_count ) {
    dc_tree_node * current_node = huffman_tree;
    uint8_t next_byte;
    bit_array * bit_arr;
    size_t bytes_read = dc_read( src_fd, &next_byte, BUFFER_SIZE );
    if ( bytes_read == 0 ) {
        fprintf( stderr, "Nothing to decompress" );
    }
    uint8_t byte = next_byte;
    bytes_read = dc_read( src_fd, &next_byte, BUFFER_SIZE );
    while ( bytes_read ) {
        bit_arr = get_bit_array( 0 );
        *bit_arr->array = byte;
        current_node = decompress_char( huffman_tree, current_node, bit_arr, CHAR_BIT );
        byte = next_byte;
        bytes_read = dc_read( src_fd, &next_byte, BUFFER_SIZE );
    }
    bit_arr = get_bit_array( 0 );
    *bit_arr->array = byte;
    current_node = decompress_char( huffman_tree, current_node, bit_arr, spill_over_bit_count );
    if ( current_node != huffman_tree ) {
        fprintf( stderr, "File was not the correct size!" );
        exit( EXIT_FAILURE );
    }
}

void huffman_decompress( int src_fd ) {
    dlinked_list * char_counts = parse_huffman_header( src_fd );
    dlinked_list * sorted_ch_counts = dlinked_quicksort( char_counts, sorting_comparator );
    dc_tree_node * huffman_tree = make_huffman_tree( sorted_ch_counts );
    size_t spillover_bit_count = get_spillover_bit_count( huffman_tree );
    decode_file( src_fd, huffman_tree, spillover_bit_count );
    free_tree_contents( &huffman_tree );
    dlinked_free_list( &char_counts );
}
