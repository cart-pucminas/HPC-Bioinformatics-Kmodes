#ifndef _GLOBAL_
#define _GLOBAL_


#include <stdio.h>
#include <stdlib.h>
#include "sequence.h"

#if USE_MPI
#include <mpi.h>
#endif

#define BIT_SIZE_OF(x) (sizeof(x) * 8)

#define DEBUG 1
#define BINARY_OUT 0
// 
// extern sequence_t *data;
// extern int *label;
// extern size_t data_size;
// extern size_t clusters;
// extern sequence_t *centroids;

extern int mpi_rank;
extern int mpi_size;

#endif
