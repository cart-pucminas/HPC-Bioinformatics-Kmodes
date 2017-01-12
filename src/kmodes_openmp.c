#include <string.h>
#include <limits.h>
#include <omp.h>

#include "global.h"
#include "power.h"
#include "kmodes.h"
#include "kmodes_common.h"


/**
* Calculate the nearests distance between the sequence and the centroids
* @param labels, pointer where the centroid labels are stored
* @param centroids, the current centroids of the clusters
* @return number of points that where moved to another centroid
*/
__OFFLOAD__MODIFIER__ size_t calculate_nearest_centroids(sequence_t *data , size_t data_size, sequence_t *centroids, size_t clusters, int *labels) {
  int delta = 0; //Number of objects has diverged in current iteration

  int nearests[data_size];
  size_t min_distances[data_size];

  // #pragma omp parallel for
  for (size_t i = 0; i < data_size; i++) {
    // logDistanceSequence(data[i]);

    min_distances[i] = UINT_MAX;
    nearests[i] = -1;

    for(size_t j = 0;j < clusters;j++) {
      unsigned int distance = dist_sequence(data[i], centroids[j]);
      if(distance < min_distances[i]) {
        nearests[i] = j;
        min_distances[i] = distance;
      }
      // logDistanceFromCluster(centroids[j], j, distance);
    }

    if(labels[i] != nearests[i]) {
      // #pragma omp atomic
      delta++;
      labels[i] = nearests[i];
    }
    // logNearestDistance(nearests[i], min_distances[i]);
  }
  return delta;
}

kmodes_result_t kmodes(kmodes_input_t input) {
  power_init();
  sequence_t *data = input.data;
  size_t data_size = input.data_size;
  size_t clusters = input.number_of_clusters;
  // printf("Threads count = %d\n", thread_count);
  printf("Execution XeonPhi Kmodes, number of clusters %zu\n", clusters);

  int *labels = (int*)calloc(data_size,sizeof(int));
  sequence_t *centroids = (sequence_t*)calloc(clusters, sizeof(sequence_t));

  #pragma offload target(mic:0)  in(data:length(data_size)) out(centroids:length(clusters)) out(labels:length(data_size))
  {

    memset (labels, -1 , data_size * sizeof(int));
    unsigned int *tmp_centroidCount = NULL;
    tmp_centroidCount = (unsigned int*)malloc(clusters * BIT_SIZE_OF(sequence_t) * sizeof(unsigned int));

    for(size_t i = 0;i < clusters;i++) {
      size_t h = i * data_size / clusters;
      sequence_t sequence = copy_sequence(data[h]);
      centroids[i] = sequence;
    }

    int pc = 0;
    int delta = 0;

    do {
      memset (tmp_centroidCount, 0 , clusters * BIT_SIZE_OF(sequence_t) * sizeof(unsigned int));
      delta = calculate_nearest_centroids(data, data_size, centroids, clusters, labels);

      for(size_t i = 0;i < data_size;i++) {
        unsigned int *tmp_centroid = &tmp_centroidCount[labels[i] * BIT_SIZE_OF(sequence_t)];
        for (size_t j=0;j<SEQ_DIM_BITS_SIZE;j++){
          // bits tmp_centroid[0] is less significative bit from sequence_t
          // bits tmp_centroid[0] = z << 0
          uint64_t mask = 1;

          if (data[i].z & (mask << j)){
            tmp_centroid[j]++;
          }
          if (data[i].y & (mask << j)){
            tmp_centroid[SEQ_DIM_BITS_SIZE + j]++;
          }
          if (data[i].x & (mask << j)){
            tmp_centroid[(2 *SEQ_DIM_BITS_SIZE) + j]++;
          }
        }
      }

      for(size_t i = 0;i < clusters; i++) {
        sequence_t seq = { 0,0,0 };

        unsigned int *tmp_centroid = &tmp_centroidCount[i* BIT_SIZE_OF(sequence_t)];

        for (size_t j = 0; j < SEQ_DIM_BITS_SIZE; j+= 4) {

          // bits tmp_centroid[0] is less significative bit from sequence_t
          // bits tmp_centroid[0] = z << 0
          unsigned int *bitCountX = &tmp_centroid[j + (SEQ_DIM_BITS_SIZE * 2)];
          unsigned int *bitCountY = &tmp_centroid[j + SEQ_DIM_BITS_SIZE];
          unsigned int *bitCountZ = &tmp_centroid[j];

          uint64_t mask = maskForMode(bitCountX[0],bitCountX[1],bitCountX[2],bitCountX[3]);
          seq.x |= (mask << j);
          mask = maskForMode(bitCountY[0],bitCountY[1],bitCountY[2],bitCountY[3]);
          seq.y |= (mask << j);
          mask = maskForMode(bitCountZ[0],bitCountZ[1],bitCountZ[2],bitCountZ[3]);
          seq.z |= (mask << (j));
        }
        centroids[i] = seq;

      }
      printf ("%d - delta = %d\n", pc ,delta);

      pc++;
    } while(delta > 0);
  }
  exit(0);
  kmodes_result_t result = {
    labels,
    centroids,
  };
  power_end();
  return result;
}
