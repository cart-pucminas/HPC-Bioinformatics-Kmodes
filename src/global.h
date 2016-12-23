#ifndef _GLOBAL_
#define _GLOBAL_

#define safe_print(...) if (mpi_rank == 0) printf(__VA_ARGS__)
#define master_only(command) if (mpi_rank == 0) { command; };

#include <stdio.h>
#include <stdlib.h>
#include "sequence.h"

#if USE_MPI
#include <mpi.h>
#endif

#define BIT_SIZE_OF(x) (sizeof(x) * 8)

#define DEBUG 0
#define BINARY_OUT 0

extern int mpi_rank;
extern int mpi_size;

#endif
