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


#include <stdio.h>
#include <stdlib.h>
#include "bwt.h"

//unsigned char block1[] = "a_asa_da_casaa";

unsigned char block1[] = "Alice was beginning to get very tired of sitting by her sister on the bank, \n"
        "and of having nothing to do: once or twice she had peeped into the book her sister was reading, \n"
        "but it had no pictures or conversations in 'and what is the use of a book,' thought Alice \n"
        "'without pictures or conversations?\n"


        "\nSo she was considering in her own mind (as well as she could, for the hot day made \n"
        "her feel very sleepy and stupid), whether the pleasure of making a daisy-chain would \n"
        "be worth the trouble of getting up and picking the daisies, when suddenly a White Rabbit \n"
        "with pink eyes ran close by her.\n"

        "\nThere was nothing so VERY remarkable in that; nor did Alice think it so VERY \n"
        "much out of the way to hear the Rabbit say to itself, 'Oh dear! Oh dear! I shall be late!' \n"
        "(when she thought it over afterwards, it occurred to her that she ought to have wondered at \n"
        "this, but at the time it all seemed quite natural); but when the Rabbit actually TOOK A WATCH \n"
        "OUT OF ITS WAISTCOAT-POCKET, and looked at it, and then hurried on, Alice started to her feet, \n"
        "for it flashed across her mind that she had never before seen a rabbit with either a waistcoat-pocket,\n"
        "or a watch to take out of it, and burning with curiosity, she ran across the field after it, and\n"
        "fortunately was just in time to see it pop down a large rabbit-hole under the hedge. \n";


unsigned char compressed_block1[4000];
unsigned char uncompressed_block1[4000];

int main(int argc, char** argv) {

    int i;
    printf("Burrows–Wheeler transform with bitmap RLE:\n");
    printf("Original: %s\n", block1);


    int size_compressed = encode(sizeof (block1), block1, sizeof (compressed_block1), compressed_block1);

    for (i = 0; i < size_compressed; i++) {
        //printf("%c", compressed_block1[i]);
    }
    printf("\n");

    printf("Size of block (compressed): %d (reduction of %f)\n",
            size_compressed, 1.0 - (size_compressed / (double) sizeof(block1)));

    int size_decompressed = decode(size_compressed, compressed_block1, sizeof (uncompressed_block1), uncompressed_block1);

    printf("\nDecoded: \n%s\n", uncompressed_block1);

    printf("\nOriginal: %d - Compressed: %d - Decompressed: %d\n", sizeof (block1), size_compressed, size_decompressed);

    return 0;


    return (EXIT_SUCCESS);
}

