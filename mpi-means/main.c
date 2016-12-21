#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <omp.h>
#include "global.h"
#include "io.h"
#include "kmeans.h"

void execute(char *filename){
   
    double begin = MPI_Wtime();
    char logfile[255];
    //Open Log file
    if(mpi_rank == 0){
        sprintf(logfile,"%s.log",filename);
        log_file = fopen(logfile,"w");

        sprintf(logfile,"%s.network",filename);
        log_network = fopen(logfile,"w");

        if(!log_file || !log_network) {
            printf("Couldn't open log file!");
            MPI_Finalize();
            exit(0);
        }

        fprintf(log_network,"Pack (bytes)\tTime\n----------------------------\n");
        fprintf(log_file,"Delta\tTime\n----------------------\n");
    }
   
    //Execute processing

    read_data(filename);

    kmeans();
    char resultFile[255];
    sprintf(resultFile,"%s.out",filename);
    write_nearest_objects(resultFile);

    if(mpi_rank == 0) {
        double end = MPI_Wtime();
        fprintf(log_file,"---\n%f",end-begin);
        fclose(log_file);
        fclose(log_network);
    }

}

int main(int argc,char **argv){
	char filename[255];
	int provider;

	//Initialize MPI
	MPI_Init_thread(&argc,&argv,MPI_THREAD_FUNNELED,&provider);
	MPI_Comm_rank(MPI_COMM_WORLD,&mpi_rank);
	MPI_Comm_size(MPI_COMM_WORLD,&mpi_size);

	if(argc != 3 && argc != 4) {
        PRINT("Invalid arguments, usage:\n");
		PRINT("\tpdtis [INPUT] [CLUSTERS]\n");
		MPI_Finalize();
		return 0;
	}
	strcpy(filename, argv[1]);
	clusters = atoi(argv[2]);
	if (argc == 4){
        for (int i = 1; i < 11; i++ ){
            sprintf(filename,"%s%d",argv[1], i);
            if (i == 10){
                clusters = atoi(argv[3]);
            }
            execute(filename);
            MPI_Barrier(MPI_COMM_WORLD);
        }
	}else{
	    execute(filename);
	}

	MPI_Finalize();

	return 0;
}
