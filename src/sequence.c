//
//  sequence.c
//  TD
//
//  Created by Guilherme Torres on 9/7/13.
//  Copyright (c) 2013 Guilherme Torres. All rights reserved.
//
#include <stdio.h>
#include <inttypes.h>
#include "sequence.h"
#include "global.h"

/**
 * @return The respective nucleotide characther for the given centroid_values
 * a = 0001
 * c = 0010
 * g = 0100
 * t = 1000
 */
unsigned char base_for_binary(unsigned char b) {
  if (b == 0b0001)
  return 'a';
  if (b == 0b0010)
  return 'c';
  if (b == 0b0100)
  return 'g';
  if (b == 0b1000)
  return 't';
  return '\0';
}

/**
 * Create an human readable represantion of the sequence last sequence part
 * The last for codes of the string represents the presence of some charecterist
 * where the code 0010 represents pre absence of this characteristic and the code
 * 0001 represents the presence of the characteristic.
 * example sequence 0001 0010 0100 1000 0001 0010 0010
 * will be return the string "acgt 100"
 */
__OFFLOAD__MODIFIER__ void create_readable_string_for_last_part(char *str, uint64_t sequence_part)
{
  char *ptr = (char*)&sequence_part;
  unsigned char *b = (unsigned char*) ptr;
  size_t size = sizeof(uint32_t); // only half of the bits will be used on Z
  // remove characteristic bits
  uint64_t threeLast = ((*ptr) & 0xFFFUL);
  *ptr >>= 12;

  unsigned char byte;
  for (int i=size;i > 0;i--)
  {
    byte = b[i] >> 4;
    *str++ = base_for_binary(byte);
    byte = b[i] &= 0xf;
    *str++ = base_for_binary(byte);
  }
  *str++ = ' ';

  byte = threeLast >> 0b1000;
  *str++ = byte == 1 ? '1' : '0';

  byte = (threeLast >> 0b0100) & 0xF;
  *str++ = byte == 1 ? '1' : '0';

  byte = threeLast & 0xF;
  *str++ = byte == 1 ? '1' : '0';

  *str++ = 0;
}

/**
 * Create an human readable represantion of the sequence part
 * example sequence 0001 0010 0100 1000 will be return the string "acgt"
 * @return The pointer to the last char added to the string
 */
char * __OFFLOAD__MODIFIER__ create_readable_string(char *str, uint64_t sequence_part) {
  char *ptr = (char*)&sequence_part;
  unsigned char *b = (unsigned char*) ptr;
  unsigned char byte;
  size_t size = sizeof(uint64_t);

  for (int i = size-1; i >= 0; i--)
  {
    byte = b[i] >> 4;
    *str++ = base_for_binary(byte);

    byte = b[i] &= 0xf;
    *str++ = base_for_binary(byte);
  }
  *str++ = ' ';
  return str++;

}

/**
 * Create an human readable representation of the sequence
 * per example: atgtacccatggacct ccagcagccagatggt gctccgag 000
 */
__OFFLOAD__MODIFIER__ char* to_human_readble_string(sequence_t seq) {
  char* string = (char *) calloc(196, sizeof(char));
  char *ptr = create_readable_string(string, seq.x);
  ptr = create_readable_string(ptr, seq.y);
  create_readable_string_for_last_part(ptr, seq.z);
  return string;
}

__OFFLOAD__MODIFIER__ void print_binary(void const * const ptr)
{
  unsigned char *b = (unsigned char*) ptr;
  unsigned char byte;
  size_t size = sizeof(uint64_t);
  int i, j;

  for (i=size-1;i>=0;i--)
  {
    for (j=7;j>=0;j--)
    {
      byte = b[i] & (1<<j);
      byte >>= j;
      printf("%u", byte);
    }
  }
}

__OFFLOAD__MODIFIER__ void print_sequence(sequence_t seq)
{
  print_binary(&seq.x);
  print_binary(&seq.y);
  print_binary(&seq.z);
}

__OFFLOAD__MODIFIER__ int dist_sequence(sequence_t seq1,sequence_t seq2){
  uint64_t xDiff = seq1.x ^ seq2.x;
  uint64_t yDiff = seq1.y ^ seq2.y;
  uint64_t zDiff = seq1.z ^ seq2.z;
  return __builtin_popcountll(xDiff) + __builtin_popcountll(yDiff) + __builtin_popcountll(zDiff);
}

__OFFLOAD__MODIFIER__ sequence_t copy_sequence(sequence_t seq) {
  sequence_t sequence = { seq.x,seq.y,seq.z };
  return sequence;
}

sequence_t sum_sequence(sequence_t seq1,sequence_t seq2) {
  sequence_t sequence = { seq1.x + seq2.x,seq1.y + seq2.y,seq1.z + seq2.z };
  return sequence;
}

sequence_t div_sequence(sequence_t seq1, uint64_t div){
  sequence_t sequence = { seq1.x / div , seq1.y / div, seq1.z / div };
  return sequence;
}
