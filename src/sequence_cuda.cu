//
//  sequence.c
//  TD
//
//  Created by Guilherme Torres on 9/7/13.
//  Copyright (c) 2013 Guilherme Torres. All rights reserved.
//

#include <stdio.h>
#include <cuda_runtime.h>
#include "sequence.h"

static size_t const sequence_part1_size = sizeof(uint64_t) * 8;
static size_t const sequence_part2_size = sequence_part1_size * 2;

__OFFLOAD__MODIFIER__ inline void create_binary_string(void const * const ptr, char *str) {
  unsigned char *b = (unsigned char*) ptr;
  unsigned char byte;
  size_t size = sizeof(unsigned long int);
  size_t count = 0;

  for (sizet_t i = size-1; i >= 0; i--)
  {
    for (sizet_t j=7; j >= 0; j--)
    {
      byte = b[i] & (1<<j);
      byte >>= j;
      str[count] = byte +'0';
      count++;
    }
  }
}

__OFFLOAD__MODIFIER__ inline char* to_string(sequence_t seq) {
  const char* string = (char *) malloc(sizeof(sequence_t) * 8);
  create_binary_string(string, &seq.x);
  create_binary_string(&string[sequence_part1_size], &seq.y);
  create_binary_string(&string[sequence_part2_size], &seq.z);
  return string;
}

__OFFLOAD__MODIFIER__ inline void print_sequence(sequence_t seq) {
  const char* string = to_string(seq);
  printf("%s\n", string);
  free(string);
}

__OFFLOAD__MODIFIER__ inline int dist_sequence(sequence_t seq1,sequence_t seq2) {
  unsigned long int xDiff = seq1.x ^ seq2.x;
  unsigned long int yDiff = seq1.y ^ seq2.y;
  unsigned long int zDiff = seq1.z ^ seq2.z;
  return __builtin_popcountl(xDiff) + __builtin_popcountl(yDiff) + __builtin_popcountl(zDiff);
}

__OFFLOAD__MODIFIER__ inline sequence_t copy_sequence(sequence_t seq) {
  sequence_t sequence = { seq.x,seq.y,seq.z };
  return sequence;
}


__OFFLOAD__MODIFIER__ inline bool get_bit(sequence_t sequence, size_t position) {

  size_t part = (position < sequence_part1_size) ? sequence.x :
    (position < sequence_part2_size) ? sequence.y : sequence.z;
  size_t part_position = position % sequence_part1_size;
  uint64_t mask = 1;
  return part & (mask << part_position);
}

__OFFLOAD__MODIFIER__ sequence_t sum_sequence(sequence_t seq1,sequence_t seq2) {
  sequence_t sequence = { seq1.x + seq2.x,seq1.y + seq2.y,seq1.z + seq2.z };
  return sequence;
}

__OFFLOAD__MODIFIER__ sequence_t inline div_sequence(sequence_t seq1, unsigned long div) {
  sequence_t sequence = { seq1.x / div , seq1.y / div, seq1.z / div };
  return sequence;
}

__OFFLOAD__MODIFIER__ inline sequence_t make_sequence(uint64_t x, uint64_t y, uint64_t z) {
  return make_ulong3(x, y, z);
}

make_sequence
