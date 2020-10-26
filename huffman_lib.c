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
#include <dc/sys/stat.h>
#include <dc/stdlib.h>
#include <dc/fcntl.h>
#include <dc/unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "stdbool.h"
#include "fcntl.h"
#include "limits.h"
#include "stdio.h"


#define TEMP_FILENAME "huffman_temp.txt"
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
    size_t * search_status = NULL;
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
    while ( dc_read( src_fd, ch, BYTES_PER_CHAR ) && dc_write( temp_fd, ch, BYTES_PER_CHAR )) {
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
        return dc_tree_create_node( dlinked_pop_head( ch_counts ));
    }
    while ( 1 < ch_counts->size ) {
        void * l_branch = dlinked_pop_head( ch_counts );
        void * r_branch = dlinked_pop_head( ch_counts );
        dc_tree_node * root = create_branch_node( l_branch, r_branch );
        size_t root_value = get_count_from_tree_node( root );
        size_t insertion_index = 0;
        int search_status = dlinked_index_of_value( ch_counts, &root_value, insertion_comparator, &insertion_index );
        if ( search_status == FAILURE || ( search_status != BAD_ARGS && insertion_index == ch_counts->size - 1 )) {
            dlinked_push( ch_counts, root );
        } else {
            dlinked_insert_value( ch_counts, insertion_index, root );
        }
    }
    return dlinked_pop_head( ch_counts );
}

void write_char( void * v_char_wrapper ) {
    char_container * char_wrapper = ( char_container * ) v_char_wrapper;
    uint8_t ch = char_wrapper->ch;
    uint32_t count = char_wrapper->count;
    write( STDOUT_FILENO, &ch, BYTES_PER_CHAR );
    write( STDOUT_FILENO, &count, BYTES_PER_CHAR_COUNT );
}

void write_char_counts( dlinked_list * char_counts ) {
    dlink_map( char_counts, write_char);
    uint8_t ch = get_char_from_tree_node(char_counts->head->content);
    write( STDOUT_FILENO, &ch, BYTES_PER_CHAR );
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
        return true;
    }
    if ( in_right_branch ) {
        path[ current_depth ] = RIGHT_BRANCH;
        return true;
    }
    return false;
}

size_t get_path_length( uint8_t * path ) {
    size_t path_length = 0;
    while ( *path != END_OF_PATH ) {
        path_length++;
        path++;
    }
    return path_length;
}

void encode_file( int src_fd, dc_tree_node * huffman_tree ) {
    uint8_t buffer = 0;
    uint8_t buffer_index = 0; // keep track of how many bits have been pushed to buffer

    uint8_t ch = 0;
    bool path_done = false;
    size_t tree_height = dc_tree_height( huffman_tree ); // maximum path length
    uint8_t path[tree_height + 1]; // extra char for termination

    size_t bytes_read = read( src_fd, &ch, BYTES_PER_CHAR );
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
        bytes_read = read( src_fd, &ch, BYTES_PER_CHAR );
    }
    // there may be some bits left in the buffer
    if ( buffer_index != 0 ) {
        buffer <<= ( uint8_t ) ( CHAR_BIT - buffer_index );
        write( STDOUT_FILENO, &buffer, 1 );
    }
}

void huffman_compress( int src_fd ) {
    int temp_fd = dc_open( TEMP_FILENAME, O_CREAT | O_WRONLY, 0640 );
    dlinked_list * char_list = get_char_counts( src_fd, temp_fd );
    dc_close( temp_fd );
    write_char_counts( char_list );
    dlinked_list * sorted_list = dlinked_quicksort( char_list, sorting_comparator );
    dlinked_free_list( &char_list );
    temp_fd = dc_open( TEMP_FILENAME, O_RDONLY );
    dc_tree_node * huffman_tree = make_huffman_tree( sorted_list );
    encode_file( temp_fd, huffman_tree );
    dc_tree_free_branch( &huffman_tree );
    dc_close( temp_fd );
    unlink( TEMP_FILENAME );
}

dlinked_list * parse_huffman_header( int src_fd, int temp_fd ) {
    dlinked_list * char_counts = dlinked_create_list();
    uint8_t header_terminator;
    uint8_t ch;
    uint32_t ch_count;
    size_t bytes_read = read( src_fd, &ch, BYTES_PER_CHAR );
    header_terminator = ch;
    bool done = false;
    while ( !done && bytes_read ) {
        write( temp_fd, &ch, BYTES_PER_CHAR );
        bytes_read = read( src_fd, &ch_count, BYTES_PER_CHAR_COUNT );
        if ( bytes_read < BYTES_PER_CHAR_COUNT ) {
            perror( "ERROR! Incorrect file size!" );
            exit( EXIT_FAILURE );
        }
        write( temp_fd, &ch_count, BYTES_PER_CHAR_COUNT );
        add_char( char_counts, ch );
        bytes_read = read( src_fd, &ch, BYTES_PER_CHAR );
        if ( ch == header_terminator ) {
            done = true;
        }
    }
    if ( !done ) {
        perror( "ERROR! Incorrect file size!" );
        exit( EXIT_FAILURE );
    }
    return char_counts;
}

uint8_t get_spillover_bit_count( dlinked_list * char_counts, dc_tree_node * huffman_tree ) {
    if ( char_counts->size == 0 ) {
        perror( "Error! No compression header!" );
        exit( EXIT_SUCCESS );
    }
    dlink * link = char_counts->head;
    size_t spillover_bits = 0;
    size_t tree_height = dc_tree_height(huffman_tree);
    while ( link ) {
        uint8_t ch = get_count_from_tree_node( link->content );
        size_t ch_count = get_count_from_tree_node( link->content );
        uint8_t path[tree_height];
        get_path_to_char( huffman_tree, ch, path, 0 );
        spillover_bits = ( spillover_bits + ch_count * get_path_length( path )) % CHAR_BIT;
        link = link->next;
    }
    return spillover_bits;
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
            write( STDOUT_FILENO, &current_node->content, BYTES_PER_CHAR );
        }
        current_node = huffman_tree;
    }
    return current_node;
}

bit_array * get_bit_array( uint8_t content ) {
    bit_array * bit_arr = bit_array_create();
    bit_array_init( bit_arr, CHAR_BIT );
    bit_array_add_byte( bit_arr, content );
    return bit_arr;
}

size_t get_file_size( int fd ) {
    struct stat st;
    dc_fstat( fd, &st );
    return st.st_size;
}

size_t get_current_position( int fd ) {
    return lseek( fd, 0, SEEK_CUR );
}

void decode_file( int src_fd, dc_tree_node * huffman_tree, uint8_t spill_over_bit_count ) {
    dc_tree_node * current_node = huffman_tree;
    uint8_t buffer;
    size_t file_size = get_file_size( src_fd );
    size_t cur_position = get_current_position( src_fd );
    bit_array * bit_arr;
    while ( cur_position < file_size ) {
        read( src_fd, &buffer, BUFFER_SIZE );
        bit_arr = get_bit_array( buffer );
        current_node = decompress_char( huffman_tree, current_node, bit_arr, CHAR_BIT );
        cur_position = get_current_position( src_fd );
    }
    read( src_fd, &buffer, BUFFER_SIZE );
    bit_arr = get_bit_array( buffer );
    decompress_char( huffman_tree, current_node, bit_arr, spill_over_bit_count );
}

void huffman_decompress( int src_fd ) {
    int temp_fd = dc_open( TEMP_FILENAME, O_CREAT | O_RDONLY, 0640 );
    dlinked_list * char_counts = parse_huffman_header( src_fd, temp_fd );
    dc_close( temp_fd );
    dc_tree_node * huffman_tree = make_huffman_tree( char_counts );
    size_t spillover_bit_count = get_spillover_bit_count( char_counts, huffman_tree );
    dlinked_free_list( &char_counts );
    temp_fd = dc_open( TEMP_FILENAME, O_RDONLY );
    decode_file( temp_fd, huffman_tree, spillover_bit_count );
    dc_tree_free_branch( &huffman_tree );
    dc_close( temp_fd );
    unlink( TEMP_FILENAME );
}
