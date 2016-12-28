#ifndef _GLOBAL_
#define _GLOBAL_
#include <stdio.h>
#include <stdlib.h>
#include "sequence.h"


#define safe_print(...) if (mpi_rank == 0) printf(__VA_ARGS__)
#define master_only(command) if (mpi_rank == 0) { command; };


#ifdef __INTEL_COMPILER
  #define __OFFLOAD__MODIFIER__ __attribute__((target(mic)))
#elif __CUDACC__
  #define __OFFLOAD__MODIFIER__ __host__ __device__
#else
  #define __OFFLOAD__MODIFIER__
#endif


#if USE_MPI
#include <mpi.h>
#endif

#define BIT_SIZE_OF(x) (sizeof(x) * 8)

#define DEBUG 0
#define BINARY_OUT 0

extern int mpi_rank;
extern int mpi_size;

#endif
