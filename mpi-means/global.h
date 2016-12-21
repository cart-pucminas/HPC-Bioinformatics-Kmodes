#ifndef _GLOBAL_
#define _GLOBAL_

#define PRINT(args ...) if(mpi_rank == 0) printf(args)
#define DIM 163

#include <stdio.h>
#include <stdlib.h>

int data_size;
int *data;
int mpi_rank = 0;
int mpi_size;
int clusters;
float *centroid;
int *label;
FILE *log_file;
FILE *log_network;

#endif
