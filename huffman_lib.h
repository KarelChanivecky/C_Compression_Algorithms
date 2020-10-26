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

#ifndef ASSIGNMENT3_HUFFMAN_LIB_H
#define ASSIGNMENT3_HUFFMAN_LIB_H

/**
 * Compress file using huffman algorithm.
 * @param src_fd an open for read file descriptor
 */
void huffman_compress( int src_fd );

/**
 * Decompress file using huffman algorithm.
 * @param src_fd an open for read file descriptor
 */
void huffman_decompress( int src_fd );

#endif //ASSIGNMENT3_HUFFMAN_LIB_H
