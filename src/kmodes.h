#ifndef _KMEANS_
#define _KMEANS_

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include "global.h"
#include "sequence.h"

typedef struct
{
  sequence_t *data;
  size_t data_size;
  size_t number_of_clusters;
} kmodes_input_t;

typedef struct
{
  int *labels;
  sequence_t *centroids;
} kmodes_result_t;

kmodes_result_t kmodes(kmodes_input_t input);

#endif
