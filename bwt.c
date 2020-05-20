/*
Simple implementation of BWT encoder with a bitmap compression algorithm
Copyright (C) 2020  Andreu Carminati
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "bwt.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static int compare_index(int a, int b, int size_in, unsigned char* in_data) {
    int i;

    for (i = 0; i < size_in; i++) {
        unsigned char ca = in_data[(a + i) % size_in];
        unsigned char cb = in_data[(b + i) % size_in];
        if (ca < cb) return 1;
        if (ca > cb) return 0;
    }
    return 0;
}

/*
 * Encoded format:
 * _______N transformed bytes________FRP___
 *|______________________________|_4 bytes_|
 * After the transformation, FRP indicates the position of the first row
 * 
 */

static int bwt(int size_in, unsigned char* in_data, unsigned char* buffer) {
    int i, j, first_row_pos = 0;
    int *index, *index_ptr;

    index = malloc(size_in * sizeof (int));

    for (i = 0; i < size_in; i++) {
        index[i] = i;
    }
    // sort rotations (insertion sort)
    for (i = 1; i < size_in; i++) {
        int sti = index[i];
        for (j = i - 1; j >= 0 && compare_index(sti, index[j], size_in, in_data); j--) {
            index[j + 1] = index[j];
            if (j == first_row_pos) { // track position of the first row
                first_row_pos++;
            }
        }
        index[j + 1] = sti; // track position of the first row
    }
    // reencode data
    for (i = 0; i < size_in; i++) {
        int pos = (index[i] + size_in - 1) % size_in;
        buffer[i] = in_data[pos];
    }
    index_ptr = (int*) (buffer + size_in);
    *index_ptr = first_row_pos;
    free(index);
    return size_in + sizeof (int);
}

static int inverse_bwt(int size_in, unsigned char* in_data, unsigned char* buffer) {
    int i, j, first_row_pos;
    unsigned char* aux_buffer;
    int *index, *T;

    first_row_pos = *(int*) (in_data + (size_in - sizeof (int)));
    size_in -= sizeof (int);
    aux_buffer = malloc(size_in);
    index = malloc(size_in * sizeof (int));
    T = malloc(size_in * sizeof (int));
    memcpy(aux_buffer, in_data, size_in);

    for (i = 0; i < size_in; i++) {
        index[i] = i;
        T[i] = i;
    }
    // sort data L and track movements (insertion sort)
    for (i = 1; i < size_in; i++) {
        unsigned char curr_char = aux_buffer[i];
        int tracked_pos = index[i];
        for (j = i - 1; j >= 0 && curr_char < aux_buffer[j]; j--) {
            aux_buffer[j + 1] = aux_buffer[j];
            index[j + 1] = index[j]; // track movements
        }
        aux_buffer[j + 1] = curr_char;
        index[j + 1] = tracked_pos; // track movements
    }
    // Build T from reordening indexes (insertion sort)
    for (i = 1; i < size_in; i++) {
        int e = index[i];
        int pos = T[i];
        for (j = i - 1; j >= 0 && e < index[j]; j--) {
            index[j + 1] = index[j];
            T[j + 1] = T[j]; // track movements back
        }
        index[j + 1] = e;
        T[j + 1] = pos; // track movements back
    }
    // Build S
    for (i = 0; i < size_in; i++) {
        int j, Ti = first_row_pos;
        for (j = 0; j < i; j++) {
            Ti = T[Ti];
        }
        buffer[size_in - 1 - i] = in_data[Ti];
    }
    free(aux_buffer);
    free(index);
    free(T);
    return size_in;
}

static void bitmap_set(unsigned char* bitmap, int bit_number, unsigned char bit) {
    int byte_number = bit_number / 8;
    int bit_offset = bit_number % 8;

    *(bitmap + byte_number) |= bit << bit_offset;
}

static unsigned char bitmap_get(unsigned char* bitmap, int bit_number) {
    int byte_number = bit_number / 8;
    int bit_offset = bit_number % 8;
    unsigned char byte = bitmap[byte_number];

    return (byte & 1 << bit_offset) >> bit_offset;
}

/*
 * Encoded format:
 * _______N unrepeated bytes_____________bitmap_______
 *|______________________________|N/8_or_(N/8)+1_bytes|
 * If we have a redundant byte, re represent the second
 * one with a '1' in the bit map in the same position. 
 * The bitmap has one bit per byte. The redundancy is
 * exposed by the Burrows Wheeler Taransform (bwt).
 * 
 */
int encode(int size_in, unsigned char* in_data, int size_buffer_out, unsigned char* buffer_out) {
    int pos = 0, compression_pos = 0, bitmap_size, bwt_size, final_size, *bitmap_offset_pos, fail = 0;
    unsigned char* buffer_data_start, *bitmap, *bwt_data;

    buffer_data_start = buffer_out + sizeof (int);
    bwt_data = malloc(size_in + sizeof (int));
    bwt_size = bwt(size_in, in_data, bwt_data);

    bitmap_size = (bwt_size % 8) == 0 ? bwt_size / 8 : (bwt_size / 8) + 1;
    bitmap = calloc(1, bitmap_size);

    while (pos < bwt_size) {
        unsigned char bit_repeat = 0;
        unsigned char next = bwt_data[pos++];
        if (next == bwt_data[pos] && pos < bwt_size) {
            bit_repeat = 1;
            pos++;
        }
        if (bitmap_size + compression_pos + sizeof (int) > size_buffer_out) {
            fail = 1; // out of memory
            break;
        }
        buffer_data_start[compression_pos] = next;
        bitmap_set(bitmap, compression_pos++, bit_repeat);
    }
    if (!fail) {
        bitmap_offset_pos = (int*) buffer_out;
        *bitmap_offset_pos = compression_pos + sizeof (int); // compression_pos also represents the number of elements
        memcpy(buffer_data_start + compression_pos, bitmap, bitmap_size);
        final_size = sizeof (int) +compression_pos + bitmap_size;
    } else {
        final_size = 0;
    }
    free(bitmap);
    free(bwt_data);
    
    return final_size;
}

int decode(int size_in, unsigned char* in_data, int size_buffer_out, unsigned char* buffer_out) {
    int bitmap_offset, size_in_data, pos = 0, to_bwt_pos = 0, final_size;
    unsigned char *bitmap, *in_data_start, *to_bwt_buffer;
    
    bitmap_offset = *((int*) in_data);
    bitmap = in_data + bitmap_offset;
    in_data_start = in_data + sizeof (int);
    size_in_data = (int) (bitmap - in_data_start);
    to_bwt_buffer = malloc(2 * size_in); // 2 is a safer constant

    while (pos < size_in_data) {
        unsigned char next = in_data_start[pos];
        unsigned char bit_repeat = bitmap_get(bitmap, pos++);
        to_bwt_buffer[to_bwt_pos++] = next;
        if (bit_repeat) {
            to_bwt_buffer[to_bwt_pos++] = next;
        }
    }
    if(size_buffer_out < to_bwt_pos){ // not enough memory
        final_size = 0;
    } else {
        final_size = inverse_bwt(to_bwt_pos, to_bwt_buffer, buffer_out);
    }
    free(to_bwt_buffer);
    return final_size;
}

