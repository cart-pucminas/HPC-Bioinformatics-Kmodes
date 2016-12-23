#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "global.h"
#include "io.h"
#include "power.h"


int mpi_rank;
int mpi_size;

void execute(char *filename, size_t number_of_clusters) {
  #if !USE_MPI
    int mpi_rank = 0;
  #endif

  if (mpi_rank == 0) {
    printf("Input file: %s\n", filename);
    #if EMMC_POWER_MEASUREMENT
    power_init();
    #endif
  }

  //Execute processing

  kmodes_input_t input = read_data(filename);
  input.number_of_clusters = number_of_clusters;

  clock_t begin = clock();
  kmodes_result_t result = kmodes(input);
  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  if (mpi_rank == 0) {
    printf("Excution time: %f seconds\n\n", time_spent);
    char resultFile[255];
    sprintf(resultFile, "%s.out", filename);
    write_nearest_objects(resultFile, input, result);
    #if EMMC_POWER_MEASUREMENT
    double power = power_end();
	  printf("power measured: %f\n", power);
    #endif
  }

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

  if(argc != 3 && argc != 4) {
    printf("Invalid arguments, usage:\n");
    printf("\tpdtis [INPUT] [CLUSTERS]\n");
    #if USE_MPI
    MPI_Finalize();
    #endif
    return 0;
  }

  strcpy(filename, argv[1]);
  size_t number_of_clusters = atoi(argv[2]);
  if (argc == 4){
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
  } else {
    execute(filename, number_of_clusters);
  }
  #if USE_MPI
  MPI_Finalize();
  #endif

  return 0;
}
