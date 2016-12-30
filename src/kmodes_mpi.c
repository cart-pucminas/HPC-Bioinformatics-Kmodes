#include <limits.h>
#include <omp.h>
#include <mpi.h>

#include "global.h"
#include "string.h"
#include "kmodes.h"
#include "kmodes_common.h"

kmodes_result_t kmodes(kmodes_input_t input) {
  safe_print("Starting kmeans mpi\n");

  size_t clusters = input.number_of_clusters;
  size_t data_size = input.data_size;
  sequence_t *data = input.data;
  int* label = (int*)calloc(data_size, sizeof(int));
  sequence_t *centroids = (sequence_t*)calloc(clusters, sizeof(sequence_t));

  unsigned int *tmp_centroidCount = (unsigned int*)malloc(clusters * BIT_SIZE_OF(sequence_t) * sizeof(unsigned int));
  unsigned int *recv_tmp_centroidCount = (unsigned int*)malloc(clusters * BIT_SIZE_OF(sequence_t) * sizeof(unsigned int));

  memset (label,-1,data_size * sizeof(int));

  for(size_t i = 0;i < clusters;i++) {
    size_t h = i * data_size / clusters;
    centroids[i] = copy_sequence(data[h]);
  }


  int pc = 0;
  long delta = 0;
  int omp_threads_count = omp_get_max_threads();
  printf("Number of OMP threads = %d in rank = %d\n",omp_threads_count, mpi_rank);
  do {

    delta = 0; //Number of objects has diverged in current iteration
    memset (tmp_centroidCount,0,clusters * BIT_SIZE_OF(sequence_t) * sizeof(unsigned int));


    //For each point...


    for(size_t i = mpi_rank; i < data_size; i+= mpi_size) {
      unsigned int omp_distances[omp_threads_count];
      unsigned int omp_nearests[omp_threads_count];

      for (size_t j = 0; j < omp_threads_count; j++){
        omp_distances[j] = UINT_MAX;
        omp_nearests[j] = -1;
      }

      // logDistanceSequence(data[i]);


      unsigned int min_distance = UINT_MAX;
      long nearest = -1; //Nearest centroid nearest

      #pragma omp parallel for
      for(size_t j = 0;j < clusters;j++) {
        int threadId = omp_get_thread_num();
        unsigned distance = dist_sequence(data[i],centroids[j]);
        if(distance < omp_distances[threadId]) {
          omp_distances[threadId] = distance;
          omp_nearests[threadId] = j;
        }
        // logDistanceFromCluster(centroids[j], j, distance);
      }

      for(int j = 0;j < omp_threads_count;j++) {
        unsigned int distance = omp_distances[j];
        if(distance < min_distance) {
          min_distance = distance;
          nearest = omp_nearests[j];
        }
      }

      if(label[i] != nearest) {
        delta++;
        label[i] = nearest;
        logNearestDistance(nearest, min_distance);
      }

      // logLabel(i, label[i]);

      unsigned int *tmp_centroid = &tmp_centroidCount[label[i] * BIT_SIZE_OF(sequence_t)];
      #pragma omp parallel for
      for (size_t j=0;j<SEQ_DIM_BITS_SIZE;j++){
        // bits tmp_centroid[0] is less significative bit from sequence_t
        // bits tmp_centroid[0] = z << 0
        uint64_t mask = 1;

        tmp_centroid[j] += data[i].z & (mask << j));
        tmp_centroid[SEQ_DIM_BITS_SIZE + j] += data[i].y & (mask << j));
        tmp_centroid[(2 *SEQ_DIM_BITS_SIZE) + j] += data[i].x & (mask << j));
      }


      // logAddSquenceToGroup(i, label[i], tmp_centroid);

    }

    MPI_Allreduce(tmp_centroidCount, recv_tmp_centroidCount, clusters * BIT_SIZE_OF(sequence_t) ,MPI_UNSIGNED , MPI_SUM,MPI_COMM_WORLD);

    sequence_t* local_centroids = (sequence_t*)calloc(clusters,sizeof(sequence_t));

    for(size_t i = mpi_rank;i < clusters;i+=mpi_size) {
      sequence_t seq = {0,0,0};

      unsigned int *tmp_centroid = &recv_tmp_centroidCount[i* BIT_SIZE_OF(sequence_t)];

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
      local_centroids[i] = seq;
    }

    MPI_Allreduce(local_centroids, centroids, clusters * sizeof(sequence_t) ,MPI_BYTE , MPI_BOR ,MPI_COMM_WORLD);
    long recv_delta = 0;
    MPI_Allreduce(&delta,&recv_delta,1,MPI_LONG,MPI_SUM,MPI_COMM_WORLD);
    delta = recv_delta;

    free(local_centroids);
    if (mpi_rank == 0) {
      safe_print ("%d - delta = %ld\n",pc,delta);
    }
    pc++;

  } while(delta > 0);
  free(tmp_centroidCount);
  free(recv_tmp_centroidCount);
  kmodes_result_t result = {
    label,
    centroids,
  };
  return result;
}
