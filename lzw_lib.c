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
#include "stdint.h"
#include "dc/stdlib.h"

#define LZW_TABLE_SIZE 4096
#define macro(args)

uint8_t get_bits(uint16_t container, uint16_t mask, uint8_t shift) {
    mask <<= shift;
    return  (container & mask) >> shift;
}

uint8_t get_eight_bits(uint16_t container, uint8_t shift) {
    return  get_bits(container, 255u, shift);
}

uint8_t get_four_bits(uint16_t container, uint8_t shift) {
    return  get_bits(container, 15u, shift);
}

uint16_t get_four_msbs(uint16_t container) {
    return get_four_bits(container, 8);
}

uint16_t get_four_lsbs(uint16_t container) {
    return get_four_bits(container, 0);
}

uint16_t get_eight_msbs(uint16_t container) {
    return get_eight_bits(container, 4);
}

uint16_t get_eight_lsbs(uint16_t container) {
    return get_eight_bits(container, 0);
}

uint8_t ** get_initialized_lzw_table() {
    uint8_t ** lzw_table = dc_malloc(sizeof(uint8_t *) * LZW_TABLE_SIZE);
    for (uint8_t i = 0; i < UINT8_MAX; i++) {
        uint8_t * ch = (uint8_t *) dc_malloc(sizeof(uint8_t) * 2);
        ch[0] = i;
        ch[1] = 0;
        lzw_table[i] = ch;
    }
    for (uint16_t i = UINT8_MAX; i < LZW_TABLE_SIZE; i++) {
        uint8_t * empty_ch = (uint8_t *) dc_malloc(sizeof(uint8_t));
        *empty_ch = 0;
        lzw_table[i] = empty_ch;
    }
    return lzw_table;
}

void free_table(uint8_t) {

}

