//
//  sequence.h
//  TD
//
//  Created by Guilherme Torres on 9/7/13.
//  Copyright (c) 2013 Guilherme Torres. All rights reserved.
//

#ifndef TD_sequence_h
#define TD_sequence_h

#include <stdint.h>
#include <inttypes.h>

#ifdef __INTEL_COMPILER
  typedef struct ulong3
  {
    uint64_t x, y, z;
  } sequence_t;
  #define __OFFLOAD__MODIFIER__ __attribute__((target(mic)))
#elif __CUDACC__
  #include <cuda_runtime.h>
  typedef struct ulong3 sequence_t;
  #define __OFFLOAD__MODIFIER__ __host__ __device__
#else
  typedef struct ulong3
  {
    uint64_t x, y, z;
  } sequence_t;
  #define __OFFLOAD__MODIFIER__
#endif



#define SEQ_DIM_BITS_SIZE sizeof(uint64_t) * 8

__OFFLOAD__MODIFIER__ void print_sequence(sequence_t seq);

__OFFLOAD__MODIFIER__ int dist_sequence(sequence_t seq1,sequence_t seq2);

__OFFLOAD__MODIFIER__ sequence_t copy_sequence(sequence_t seq);

__OFFLOAD__MODIFIER__ sequence_t sum_sequence(sequence_t seq1,sequence_t seq2);

__OFFLOAD__MODIFIER__ sequence_t div_sequence(sequence_t seq1, uint64_t div);


#endif
