#include "kmeans.h"
#if defined(ENABLE_OPENMP)
#include <omp.h>
#else
typedef int omp_int_t;
inline omp_int_t omp_get_thread_num() { return 0;}
inline omp_int_t omp_get_max_threads() { return 1;}
#endif

void kmeans() {
    long delta,recv_delta; //Number of objects has diverged in current iteration
    
    int *count = calloc(clusters,sizeof(int));
    int *recv_count = calloc(clusters,sizeof(int));
    label = calloc(data_size,sizeof(int));
    int *send_label = calloc(data_size,sizeof(int));
    float *recv_centroid = calloc(clusters * DIM,sizeof(float));
    float *send_centroid = calloc(clusters * DIM,sizeof(float));
    float *send_tmp_centroid = calloc(clusters * DIM,sizeof(float));
    centroid = calloc(clusters * DIM,sizeof(float));
    
    //Initialize centroids
#pragma omp parallel for
    for(long i = 0;i < clusters;i++) {
        long h = i * data_size / clusters;
        
        for(long j = 0;j < DIM;j++){
            centroid[i*DIM+j] = data[h*DIM+j];
        }
    }
    
    //Initialize labels
#pragma omp parallel for
    for(size_t i = mpi_rank;i < data_size;i+= mpi_size) {
        send_label[i] = -1;
    }
    int pc = 0;
    do {
        double begin = MPI_Wtime();
        
        //Initialize tmp variables
        delta = 0;
        
#pragma omp parallel for
        for(long i = 0;i < clusters;i++) {
            for(long j = 0;j < DIM;j++) {
                send_centroid[i*DIM+j] = 0;
                send_tmp_centroid[i*DIM+j]=0;
            }
            
            count[i] = 0;
        }
        
        
        //For each point...
        for(long i = mpi_rank;i < data_size;i+=mpi_size) {
            
            int omp_threads_count = omp_get_max_threads();
            float omp_distances[omp_threads_count];
            long omp_nearests[omp_threads_count];
            for (int i = 0; i < omp_threads_count; i++){
                omp_distances[i] = FLT_MAX;
                omp_nearests[i] = -1;
            }
            
            float min_distance = FLT_MAX;
            long nearest = -1;
            
            //for each centroid, find the nearest...
#pragma omp parallel for
            for(long j = 0;j < clusters;j++) {
                float distance = 0;
                int threadId = omp_get_thread_num();
                for(long k=0;k < DIM;k++) {
                    distance += powf(data[i*DIM+k] - centroid[j*DIM+k],2);
                }
                
                if(distance < omp_distances[threadId]) {
                    omp_distances[threadId] = distance;
                    omp_nearests[threadId] = j;
                }
            }
            
            for(int j = 0;j < omp_threads_count;j++) {
                float distance = omp_distances[j];
                if(distance < min_distance) {
                    min_distance = distance;
                    nearest = omp_nearests[j];
                }
            }
            
            if(send_label[i] != nearest) {
                delta++;
                send_label[i] = nearest;
            }
            
            //Update count and tmp_centroid
            count[send_label[i]]++;
            
            for(long j = 0;j < DIM;j++) {
                send_centroid[send_label[i]*DIM+j] += data[i*DIM+j];
            }
        }
        
        double trans_init = MPI_Wtime();
        MPI_Allreduce(send_centroid,recv_centroid,clusters*DIM,MPI_FLOAT,MPI_SUM,MPI_COMM_WORLD);
        MPI_Allreduce(count,recv_count,clusters,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
        double trans_end = MPI_Wtime();
        
        if(mpi_rank == 0) {
            fprintf(log_network,"%ld\t\t%f\n",(sizeof(float)*clusters*DIM)+(clusters*sizeof(int)),trans_end - trans_init);
        }
        
#pragma omp parallel for \
shared(send_tmp_centroid)
        for(long i = mpi_rank;i < clusters;i+=mpi_size) {
            for(long j = 0;j < DIM;j++) {
                send_tmp_centroid[i*DIM+j] = recv_count[i] > 0?recv_centroid[i*DIM+j] / recv_count[i] : 0;
            }
        }
        
        trans_init = MPI_Wtime();
        MPI_Allreduce(send_tmp_centroid,centroid,clusters*DIM,MPI_FLOAT,MPI_MAX,MPI_COMM_WORLD);
        MPI_Allreduce(&delta,&recv_delta,1,MPI_LONG,MPI_SUM,MPI_COMM_WORLD);
        trans_end = MPI_Wtime();
        
        double end = MPI_Wtime();
        
        if(mpi_rank == 0) {
            fprintf(log_network,"%ld\t\t%f\n",(sizeof(float)*clusters*DIM)+sizeof(long),trans_end - trans_init);
            fprintf(log_file,"%ld\t%f\n",recv_delta,end-begin);
        }
        printf ("%d - delta = %lu\n",pc,delta);
        
        pc++;
        
    }while(recv_delta > 0);
    
    double trans_init = MPI_Wtime();
    MPI_Reduce(send_label,label,data_size,MPI_INT,MPI_MAX,0,MPI_COMM_WORLD);
    double trans_end = MPI_Wtime();
    
    if(mpi_rank == 0) {
        fprintf(log_network,"%ld\t\t%f\n",(sizeof(float)*clusters*DIM)+sizeof(long),trans_end - trans_init);			
    }
}
