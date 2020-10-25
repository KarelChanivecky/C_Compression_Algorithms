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
#include <dc_utils/dc_tree.h>
#include <dc/stdlib.h>
#include <dc/fcntl.h>
#include <dc/unistd.h>
#include "stdbool.h"
#include "fcntl.h"
#include "limits.h"

#define TEMP_FILENAME "huffman_temp.txt"
#define BYTES_PER_CYCLE 1
#define LEFT_BRANCH 0
#define RIGHT_BRANCH 1
#define END_OF_PATH 2

struct char_container_s {
    uint8_t ch;
    uint32_t count;
};
typedef struct char_container_s char_container;

char_container * create_char_container( uint8_t ch ) {
    char_container * new_char_data = ( char_container * ) dc_malloc( sizeof( struct char_container_s ));
    new_char_data->ch = ch;
    new_char_data->count = 1;
    return new_char_data;
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
    size_t * search_status;
    size_t index = dlinked_index_of_value( char_counts, ch_wrapper, has_value_comparator, search_status );
    if ( *search_status == SUCCESS ) {
        char_container * tgt_ch = dlinked_get_value( char_counts, index );
        tgt_ch->count++;
    } else {
        dlinked_push( char_counts, dc_tree_create_node( ch_wrapper ));
    }
}

dlinked_list * get_char_counts( int src_fd, int temp_fd ) {
    dlinked_list * char_counts = dlinked_create_list();
    uint8_t * ch = 0;
    while ( dc_read( src_fd, ch, BYTES_PER_CYCLE ) && dc_write( temp_fd, ch, BYTES_PER_CYCLE )) {
        add_char( char_counts, *ch );
    }
    return char_counts;
}

int compare_uint8_t( uint8_t lhs, uint8_t rhs ) {
    if ( lhs < rhs ) {
        return LHS_SMALLER;
    } else if ( lhs == rhs ) {
        return LHS_EQUAL;
    }
    return LHS_EQUAL;
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
    if ( c_el >= c_key ) {
        return true;
    }
    return false;
}

dc_tree_node * make_huffman_tree( dlinked_list * ch_counts ) {
    if ( ch_counts->size == 0 ) {
        return NULL;
    }
    if ( ch_counts->size == 1 ) {
        return dc_tree_create_node( dlinked_pop_head( ch_counts ));
    }
    while ( ch_counts->size > 1 ) {
        void * l_branch = dlinked_pop_head( ch_counts );
        void * r_branch = dlinked_pop_head( ch_counts );
        dc_tree_node * root = create_branch_node( l_branch, r_branch );
        size_t root_value = get_count_from_tree_node( root );
        size_t insertion_index = 0;
        int search_status = dlinked_index_of_value( ch_counts, root_value, insertion_comparator, &insertion_index );
        if ( search_status == FAILURE || ( search_status != BAD_ARGS && insertion_index == ch_counts->size - 1 )) {
            dlinked_push( ch_counts, root );
        } else {
            dlinked_insert_value( ch_counts, insertion_index, root );
        }
    }
    return dlinked_pop_head( ch_counts );
}

void write_char( void * v_char_wrapper, void * _ ) {
    char_container * char_wrapper = ( char_container * ) v_char_wrapper;
    uint8_t ch = char_wrapper->ch;
    uint32_t count = char_wrapper->count;
    write( STDOUT_FILENO, &ch, 1 );
    write( STDOUT_FILENO, &count, 4 );
}

void write_huffman_tree( dc_tree_node * huffman_tree ) {
    dc_tree_map( huffman_tree, write_char, NULL);
    char_container * char_wrapper = ( char_container * ) huffman_tree->content;
    uint8_t ch = char_wrapper->ch;
    write( STDOUT_FILENO, &ch, 1 );
}

bool get_path_to_char( dc_tree_node * tree, uint8_t ch, uint8_t * path, size_t current_depth ) {
    if ( !tree ) {
        return false;
    }
    char_container * char_wrapper = ( char_container * ) tree->content;
    if ( char_wrapper->ch == ch ) {
        path[ current_depth + 1 ] = END_OF_PATH;
        return true;
    }
    bool in_left_branch = get_path_to_char( tree->left, ch, path, current_depth + 1 );
    bool in_right_branch = get_path_to_char( tree->right, ch, path, current_depth + 1 );
    if ( in_left_branch ) {
        path[ current_depth ] = LEFT_BRANCH;
    } else if ( in_right_branch ) {
        path[ current_depth ] = RIGHT_BRANCH;
    }
}

void encode_file( int src_fd, dc_tree_node * huffman_tree ) {
    uint8_t buffer = 0;
    uint8_t buffer_index = 0; // keep track of how many bits have been pushed to buffer

    uint8_t ch = 0;
    bool path_done = false;
    size_t tree_height = dc_tree_height( huffman_tree ); // maximum path length
    uint8_t path[tree_height + 1]; // extra char for termination

    size_t bytes_read = read( src_fd, &ch, BYTES_PER_CYCLE );
    while ( 0 < bytes_read ) { // while we haven't read the whole file
        get_path_to_char( huffman_tree, ch, path, 0 );
        size_t path_index = 0;

        while ( !path_done ) {
            // while we have not completely processed the current path
            // write path
            while ( path[ path_index ] != END_OF_PATH && buffer < CHAR_BIT ) {
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
                buffer_index = 0;
            }
        }
        bytes_read = read( src_fd, &ch, BYTES_PER_CYCLE );
    }
    // there may be some bits left in the buffer
    if (buffer_index != 0 ) {
        buffer <<= (uint8_t)(CHAR_BIT - buffer_index);
        write( STDOUT_FILENO, &buffer, 1 );
    }
}

void huffman_compress( int src_fd ) {
    int temp_fd = dc_open( TEMP_FILENAME, O_CREAT | O_RDONLY, 0640 );
    dlinked_list * char_list = get_char_counts( src_fd, temp_fd );
    dc_close(temp_fd);
    dlinked_list * sorted_list = dlinked_quicksort( char_list, sorting_comparator );
    dlinked_free_list( &char_list );
    temp_fd = dc_open( TEMP_FILENAME, O_RDONLY );
    dc_tree_node * huffman_tree = make_huffman_tree( sorted_list );
    write_huffman_tree( huffman_tree );
    encode_file( temp_fd, huffman_tree );
    dc_tree_free_branch( &huffman_tree );
    dc_close(temp_fd);
    unlink( TEMP_FILENAME );
}

dlinked_list * parse_huffman_header( int src_fd, int temp_fd) {

}

void decode_file( int src_fd, dc_tree_node * huffman_tree ) {

}

void huffman_decompress( int src_fd ) {
    int temp_fd = dc_open( TEMP_FILENAME, O_CREAT | O_RDONLY, 0640 );
    dlinked_list * char_counts = parse_huffman_header( src_fd, temp_fd );
    dc_close(temp_fd);
    dc_tree_node * huffman_tree = make_huffman_tree( char_counts );
    dlinked_free_list( &char_counts );
    temp_fd = dc_open( TEMP_FILENAME, O_RDONLY );
    decode_file( temp_fd, huffman_tree );
    dc_tree_free_branch( &huffman_tree );
    dc_close(temp_fd);
    unlink( TEMP_FILENAME );
}
