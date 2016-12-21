#include "global.h"
#include "string.h"
#include "power.h"
#include "limits.h"
#include "kmodes.h"
#include "omp.h"

inline unsigned int __attribute__((target(mic))) maskForMode(unsigned int x,unsigned int y,unsigned int z,unsigned int w){
  unsigned int max = x > y ? x : y;
  max = z > max ? z : max;
  max = w > max ? w : max;
  unsigned int mask = 0;

  if (max == x){
    mask |= 1;
  }
  if (max == y){
    mask |= 2; // 010
  }
  if (max == z){
    mask |= 4; // 0100
  }
  if (max == w){
    mask |= 8; // 0100
  }
  return mask;
}

kmodes_result_t kmodes(kmodes_input_t input) {
  power_init();
  sequence_t *data = input.data;
  size_t data_size = input.data_size;
  size_t clusters = input.number_of_clusters;
  // printf("Threads count = %d\n", thread_count);
  printf("Execution XeonPhi Kmodes, number of clusters %zu\n", clusters);
  int *label = (int*)calloc(data_size,sizeof(int));
  sequence_t *centroids = (sequence_t*)calloc(clusters, sizeof(sequence_t));

  #pragma offload target(mic:0)  in(data:length(data_size)) out(centroids:length(clusters)) out(label:length(data_size))
  {

    long delta; //Number of objects has diverged in current iteration
    unsigned int *tmp_centroidCount = NULL;
    tmp_centroidCount = (unsigned int*)malloc(clusters * BIT_SIZE_OF(sequence_t) * sizeof(unsigned int));

    memset (label,-1,data_size * sizeof(int));

    for(size_t i = 0;i < clusters;i++) {
      size_t h = i * data_size / clusters;
      sequence_t sequence = copy_sequence(data[h]);
      centroids[i] = sequence;
    }

    int pc = 0;
    int nearests[data_size];
    unsigned int min_distances[data_size];

    do {
      delta = 0;
      memset (tmp_centroidCount,0,clusters * BIT_SIZE_OF(sequence_t) * sizeof(unsigned int));

      //#pragma omp parallel for
      for(size_t i = 0;i < data_size;i++) {
        // printf("pc = %d, i = %d\n", pc, i);
        min_distances[i] = UINT_MAX;
        nearests[i] = -1;

        for(size_t j = 0;j < clusters;j++) {
          unsigned int distance = dist_sequence(data[i],centroids[j]);
          // printf("Distance = %d\n", distance);
          if(distance < min_distances[i]) {
            nearests[i] = j;
            min_distances[i] = distance;
          }
        }

        if(label[i] != nearests[i]) {
          #pragma omp atomic
          delta++;
          label[i] = nearests[i];
        }


      }

      for(size_t i = 0;i < data_size;i++) {
        unsigned int *tmp_centroid = &tmp_centroidCount[label[i] * BIT_SIZE_OF(sequence_t)];
        for (size_t j=0;j<SEQ_DIM_BITS_SIZE;j++){
          // bits tmp_centroid[0] is less significative bit from sequence_t
          // bits tmp_centroid[0] = z << 0
          unsigned long int mask = 1;
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

      for(size_t i = 0;i < clusters;i++) {
        sequence_t seq = { 0,0,0 };

        unsigned int *tmp_centroid = &tmp_centroidCount[i* BIT_SIZE_OF(sequence_t)];

        for (size_t j = 0; j < SEQ_DIM_BITS_SIZE; j+= 4) {

          // bits tmp_centroid[0] is less significative bit from sequence_t
          // bits tmp_centroid[0] = z << 0
          unsigned int *bitCountX = &tmp_centroid[j + (SEQ_DIM_BITS_SIZE * 2)];
          unsigned int *bitCountY = &tmp_centroid[j + SEQ_DIM_BITS_SIZE];
          unsigned int *bitCountZ = &tmp_centroid[j];

          unsigned long int mask = maskForMode(bitCountX[0],bitCountX[1],bitCountX[2],bitCountX[3]);
          seq.x |= (mask << j);
          mask = maskForMode(bitCountY[0],bitCountY[1],bitCountY[2],bitCountY[3]);
          seq.y |= (mask << j);
          mask = maskForMode(bitCountZ[0],bitCountZ[1],bitCountZ[2],bitCountZ[3]);
          seq.z |= (mask << (j));
        }
        centroids[i] = seq;

      }
      printf ("%d - delta = %ld\n",pc,delta);
      pc++;
    } while(delta > 0);
  }
  kmodes_result_t result = {
    label,
    centroids,
  };
  power_end();
  return result;
}
