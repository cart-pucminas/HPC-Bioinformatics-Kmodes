#include "global.h"
#include "string.h"
#include "kmodes.h"

#include <limits.h>
#include <omp.h>
#include <mpi.h>

unsigned int maskForMode(unsigned int x,unsigned int y,unsigned int z,unsigned int w ){
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

void logDistanceSequence(sequence_t sequence){
  #if DEBUG
  printf("Calculating distance for sequence: ");
  print_sequence(sequence);
  printf("\n\n");
  #endif
}

void logDistanceFromCluster(sequence_t sequence, int cluster, int distance){
  #if DEBUG
  printf("Distance from cluster %d\n", cluster);
  print_sequence(sequence);
  printf("\n");
  printf("Distance =: %d\n\n",distance);
  #endif
}

void logNearestDistance(int nearest, int min_distance) {
  #if DEBUG
  printf("Shortest distance is cluster %d",nearest);
  printf("\n");
  printf("Distance = %d\n\n",min_distance);
  #endif
}

void logLabel(unsigned int i, int label) {
  #if DEBUG
  printf("label[%u] = %d\n",i, label);
  #endif
}

void logAddSquenceToGroup(int sequenceIndex, int cluster, const unsigned int* centroid_values){
  #if DEBUG
  printf("Added sequence %d to cluster %d \n",sequenceIndex , cluster);
  printf("Group new values are:\n");
  for (int j=BIT_SIZE_OF(sequence_t)-1;j>=0; j--) {
    printf("%d",centroid_values[j]);
  }
  printf("\n\n");
  #endif
}


kmodes_result_t kmodes(kmodes_input_t input) {
  printf("Starting kmeans mpi\n");

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
  printf("Number of OMP threads = %d\n",omp_threads_count);
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


      // logAddSquenceToGroup(i, label[i], tmp_centroid);

    }

    // AQUI FAZER O REDUCE DO tmp_centroidCount
    MPI_Allreduce(tmp_centroidCount, recv_tmp_centroidCount, clusters * BIT_SIZE_OF(sequence_t) ,MPI_UNSIGNED , MPI_SUM,MPI_COMM_WORLD);

    #if DEBUG
    for (size_t k=0;k<clusters;k++) {
      unsigned int *tmp_centroid = &recv_tmp_centroidCount[k * BIT_SIZE_OF(sequence_t)];
      for (int x=BIT_SIZE_OF(sequence_t)-1;x>=0; x--) {
        printf("%d",tmp_centroid[x]);
      }
      printf("\n");
    }
    #endif

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
      #if DEBUG
      print_sequence(local_centroids[i]);
      printf("\n");
      #endif
    }

    MPI_Allreduce(local_centroids, centroids, clusters * sizeof(sequence_t) ,MPI_BYTE , MPI_BOR ,MPI_COMM_WORLD);
    long recv_delta = 0;
    MPI_Allreduce(&delta,&recv_delta,1,MPI_LONG,MPI_SUM,MPI_COMM_WORLD);
    delta = recv_delta;

    if (mpi_rank == 0) {
      printf ("%d - delta = %ld\n",pc,delta);
    }
    pc++;

  } while(delta > 0);
  kmodes_result_t result = {
    label,
    centroids,
  };
  return result;
}
