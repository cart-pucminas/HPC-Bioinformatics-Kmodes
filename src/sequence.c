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
  unsigned long int xDiff = seq1.x ^ seq2.x;
  unsigned long int yDiff = seq1.y ^ seq2.y;
  unsigned long int zDiff = seq1.z ^ seq2.z;
  return __builtin_popcountl(xDiff) + __builtin_popcountl(yDiff) + __builtin_popcountl(zDiff);
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
