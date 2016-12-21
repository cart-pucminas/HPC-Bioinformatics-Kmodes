#define _GNU_SOURCE
#include "io.h"
#include <stdio.h>


void read_data(char *file) {
	size_t line_size = 255;
	double trans_init,trans_end;

	if(mpi_rank == 0){
		FILE *in = fopen(file,"r");
		char *line = calloc(255,sizeof(char));
		int current_line = 0;
		
		if(!in) {
			printf("Invalid file!\n");
			MPI_Finalize();
			exit(0);
		}
		
		//Count lines
		data_size = 0;
		rewind(in);
		
		while(!feof(in)) {
			getline(&line,&line_size,in);
            if(line[0] != '\n'){
				data_size++;
			}
        }
		
		//Read objects
		data = calloc(data_size*DIM,sizeof(int));
		rewind(in);
		
		while(!feof(in)) {
			getline(&line,&line_size,in);
			
			if(strlen(line) > 0 && line[0] != '\n'){
				for(int i = 0;i < DIM;i++ ){
					data[current_line*DIM+i] = line[i]-'0';
				}
				current_line++;
			}
		}
		
		fclose(in);
	}	
	
	trans_init = MPI_Wtime();
	MPI_Bcast(&data_size,1,MPI_INT,0,MPI_COMM_WORLD);
	trans_end = MPI_Wtime();

	if(mpi_rank == 0) {
		fprintf(log_network,"%ld\t\t%f\n",sizeof(int),trans_end - trans_init);			
	}

	if(mpi_rank != 0) {
		data = calloc(data_size*DIM,sizeof(int));
	}
	
	trans_init = MPI_Wtime();
	MPI_Bcast(data,data_size*DIM,MPI_INT,0,MPI_COMM_WORLD);
	trans_end = MPI_Wtime();

	if(mpi_rank == 0) {
		fprintf(log_network,"%ld\t\t%f\n",sizeof(int) * DIM*data_size,trans_end - trans_init);			
	}
}

void write_nearest_objects(char *file) {
	if(mpi_rank == 0) {
	
		FILE *out = fopen(file,"w");
		long i,j,k,nearest;
		float distance,min_distance;
	
		if(!out) {
			printf("Invalid file!\n");
			MPI_Finalize();
			exit(0);
		}
	
		//For each centroid
		for(i = 0;i < clusters;i++) {
			nearest = -1; 
			min_distance = FLT_MAX;
		
			//Find the nearest object
			for(j = 0;j < data_size;j++) {
				if(label[j] != i) {
					continue;
				}
			
				distance = 0;
			
				for(k = 0;k < DIM;k++) {
					distance += powf(data[j*DIM+k] - centroid[i*DIM+k],2);
				}
			
				distance = sqrtf(distance);
			
				if(distance < min_distance) {
					nearest = j;
					min_distance = distance;
				}
			}
		
			if(nearest == -1) {
				nearest = i * data_size/clusters;
			}
		
			for(j = 0;j < DIM;j++) {
				fprintf(out,"%d",data[nearest*DIM+j]);
			}
			
			
			fprintf(out,"\n");
		}
	
		fclose(out);
	}
}
