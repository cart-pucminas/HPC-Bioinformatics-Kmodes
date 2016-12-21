#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "io.h"


int mpi_rank;
int mpi_size;

void execute(char *filename, size_t number_of_clusters) {

  #if USE_MPI
  FILE *log_file;
  char logfile[255];
  double begin = MPI_Wtime();
  //Open Log file
  if(mpi_rank == 0){
    sprintf(logfile,"%s.log",filename);
    log_file = fopen(logfile,"w");
  }
  #endif
  //Execute processing

  kmodes_input_t input = read_data(filename);
  input.number_of_clusters = number_of_clusters;
  kmodes_result_t result = kmodes(input);

  char resultFile[255];
  sprintf(resultFile, "%s.out", filename);
  write_nearest_objects(resultFile, input, result);

  free(input.data);
  free(result.labels);
  free(result.centroids);

  #if USE_MPI
  if(mpi_rank == 0) {
    double end = MPI_Wtime();
    fprintf(log_file,"---\n%f",end-begin);
    fclose(log_file);
  }
  #endif
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
