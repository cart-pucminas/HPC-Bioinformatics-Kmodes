#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "global.h"
#include "io.h"
#include "power.h"


int mpi_size;
int mpi_rank = 0;

void execute(char *filename, size_t number_of_clusters) {

  //Execute processing

  kmodes_input_t input = read_data(filename);
  input.number_of_clusters = number_of_clusters;

  struct timeval  tv1, tv2;
  gettimeofday(&tv1, NULL);
  
  kmodes_result_t result = kmodes(input);
  
  gettimeofday(&tv2, NULL);
  safe_print("Total time = %f seconds\n",
         (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
         (double) (tv2.tv_sec - tv1.tv_sec));

  char resultFile[255];
  master_only(sprintf(resultFile, "%s.out", filename));
  master_only(write_nearest_objects(resultFile, input, result));

  free(input.data);
  free(result.labels);
  free(result.centroids);

}

void init_mpi(int argc,char **argv){
  #if USE_MPI
  int provider;
  MPI_Init_thread(&argc,&argv,MPI_THREAD_FUNNELED,&provider);
  MPI_Comm_rank(MPI_COMM_WORLD,&mpi_rank);
  MPI_Comm_size(MPI_COMM_WORLD,&mpi_size);

  printf("Initializing using MPI. Rank: %d\n", mpi_rank);
  #endif
}

int main(int argc,char **argv) {
  char filename[255];
  init_mpi(argc, argv);

  #if EMMC_POWER_MEASUREMENT
  master_only(power_init());
  #endif

  if(argc != 3 && argc != 4) {
    safe_print("Invalid arguments, usage:\n");
    safe_print("\tpdtis [INPUT] [CLUSTERS]\n");
    #if USE_MPI
    MPI_Finalize();
    #endif
    return 0;
  }

  strcpy(filename, argv[1]);
  size_t number_of_clusters = atoi(argv[2]);

  if (argc == 4) {
    struct timeval  tv1, tv2;
    gettimeofday(&tv1, NULL);

    for (int i = 1; i < 11; i++ ){
      sprintf(filename,"%s%d",argv[1], i);

      if (i == 10){
        number_of_clusters = atoi(argv[3]);
      }

      execute(filename, number_of_clusters);
      #if USE_MPI
      MPI_Barrier(MPI_COMM_WORLD);
      #endif

    } // end for
    gettimeofday(&tv2, NULL);
    safe_print ("Total time = %f seconds\n",
         (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
         (double) (tv2.tv_sec - tv1.tv_sec));
  } else {
    execute(filename, number_of_clusters);
  }
  #if USE_MPI
  MPI_Finalize();
  #endif
  #if EMMC_POWER_MEASUREMENT
  double power = 0;
  master_only(power = power_end());
  safe_print("power measured: %f\n", power);
  #endif

  return 0;
}
