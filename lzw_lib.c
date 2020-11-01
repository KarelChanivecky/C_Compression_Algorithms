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

uint16_t get_four_lsbs(uint16_t container) {
    return get_eight_bits(container, 0);
}

