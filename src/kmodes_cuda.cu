#include "global.h"
#include "sequence.h"

extern "C" {
  #include "kmodes.h"
}

#include <string.h>
#include <cuda.h>

#define checkCuda(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort=true)
{
  if (code != cudaSuccess)
  {
    fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
    cudaDeviceReset();
    if (abort) exit(code);
  }
}

__device__ unsigned int cuda_delta = 0;

__device__ unsigned int maskForMode(unsigned int x,
  unsigned int y,
  unsigned int z,
  unsigned int w ){
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


  __global__ void update_centroids(const sequence_t *data,
    sequence_t *centroids,
    unsigned int * tmp_centroidCount,
    unsigned int numClusters){
      int i = blockIdx.x * blockDim.x  + threadIdx.x;

      if (i < numClusters){
        sequence_t seq = make_ulong3(0,0,0);
        unsigned int *tmp_centroid = &tmp_centroidCount[i* BIT_SIZE_OF(sequence_t)];
        for (int j=0;j<SEQ_DIM_BITS_SIZE;j+=4)
        {

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
          seq.z |= (mask << j);
        }
        centroids[i] = seq;
      }
    }

    __global__ void kmeans_kernel(const sequence_t *data,
      const sequence_t *centroids,
      int * membership,
      unsigned int * tmp_centroidCount,
      unsigned int n,
      unsigned int numClusters )
      {

        int index = blockIdx.x * blockDim.x  + threadIdx.x;
        if (index < n){

          unsigned int min_distance = UINT_MAX;
          long nearest = -1;

          for (int i = 0; i < numClusters; i++){
            sequence_t centroid = centroids[i];
            unsigned int distance = __popcll(centroid.x ^ data[index].x) +
            __popcll(centroid.y ^ data[index].y) +
            __popcll(centroid.z ^ data[index].z);
            if(distance < min_distance) {
              nearest = i;
              min_distance = distance;
            }
          }

          if(membership[index] != nearest) {
            membership[index]=nearest;
            atomicInc(&cuda_delta,n*2);
          }

          unsigned int *tmp_centroid = &tmp_centroidCount[membership[index] * BIT_SIZE_OF(sequence_t)];
          for (unsigned int j=0;j<SEQ_DIM_BITS_SIZE;j++)  {
            // bits tmp_centroid[0] is less significative bit from sequence_t
            // bits tmp_centroid[0] = z << 0
            unsigned long int mask = 1;
            if (data[index].z & (mask << j)){
              atomicInc(&tmp_centroid[j],n);
            }
            if (data[index].y & (mask << j)){
              atomicInc(&tmp_centroid[SEQ_DIM_BITS_SIZE + j],n);
            }
            if (data[index].x & (mask << j)){
              atomicInc(&tmp_centroid[(2 *SEQ_DIM_BITS_SIZE)+j],n);
            }
          }
        }
      }


      void checkCudaMemory(){
        // show memory usage of GPU

        size_t free_byte ;
        size_t total_byte;
        checkCuda(cudaMemGetInfo( &free_byte, &total_byte ));
        double free_db = (double)free_byte ;
        double total_db = (double)total_byte ;
        double used_db = total_db - free_db ;
        printf("GPU memory usage: used = %f, free = %f MB, total = %f MB\n",
        used_db/1024.0/1024.0, free_db/1024.0/1024.0, total_db/1024.0/1024.0);
      }

      extern "C"
      kmodes_result_t kmodes(kmodes_input_t input) {
        printf("%s\n", "Execution cuda kmodes");
        //checkCudaMemory();

        unsigned int delta=0; //Number of objects has diverged in current iteration

        size_t clusters = input.number_of_clusters;
        size_t data_size = input.data_size;
        sequence_t *data = input.data;
        int* label = (int*)calloc(data_size, sizeof(int));
        sequence_t *centroids = (sequence_t*)calloc(clusters, sizeof(sequence_t));


        // cuda variables
        sequence_t * cuda_data = NULL;
        sequence_t * cuda_centroids = NULL;
        int *cuda_membership = NULL;
        unsigned int *cuda_tmp_centroidCount = NULL;

        //memory arrays size
        const unsigned int cuda_tmp_centroidCountSize = clusters * BIT_SIZE_OF(sequence_t) *sizeof(unsigned int);

        const unsigned int threadsPerBlock = 128;
        const unsigned int numBlocks = (data_size + threadsPerBlock - 1) / threadsPerBlock;
        const unsigned int numBlocks2 = (clusters + threadsPerBlock - 1) / threadsPerBlock;

        for(unsigned int i = 0;i < clusters;i++) {
          int h = i * data_size / clusters;
          centroids[i] = make_ulong3(data[h].x,data[h].y,data[h].z);
        }

        memset (label,-1,data_size * sizeof(int));

        checkCuda(cudaMalloc(&cuda_data, data_size * sizeof(sequence_t)));
        checkCuda(cudaMalloc(&cuda_centroids, clusters * sizeof(sequence_t)));
        checkCuda(cudaMalloc(&cuda_membership, data_size * sizeof(int)));
        checkCuda(cudaMalloc(&cuda_tmp_centroidCount,cuda_tmp_centroidCountSize));
        cudaDeviceSynchronize();

        checkCuda(cudaMemcpy(cuda_data,data, data_size *sizeof(sequence_t) , cudaMemcpyHostToDevice));
        checkCuda(cudaMemcpy(cuda_centroids, centroids, clusters *sizeof(sequence_t) , cudaMemcpyHostToDevice));
        checkCuda(cudaMemset(cuda_membership,-1,data_size * sizeof(int)));
        cudaDeviceSynchronize();

        printf("Num blocks: %d Num threads: %d total %d\n",numBlocks,threadsPerBlock,threadsPerBlock * numBlocks);

        do {
          delta = 0;

          checkCuda(cudaMemset(cuda_tmp_centroidCount,0,cuda_tmp_centroidCountSize));
          checkCuda(cudaMemcpyToSymbol(cuda_delta, &delta,sizeof(unsigned int),0,cudaMemcpyHostToDevice));
          cudaDeviceSynchronize();

          // first kernel
          kmeans_kernel <<< numBlocks,threadsPerBlock>>>(cuda_data,
            cuda_centroids,
            cuda_membership,
            cuda_tmp_centroidCount,
            data_size,
            clusters);
            checkCuda(cudaMemcpy(label,cuda_membership, data_size *sizeof(int) , cudaMemcpyDeviceToHost));
            checkCuda(cudaGetLastError());
            cudaDeviceSynchronize();
            // second kernel
            update_centroids<<<numBlocks2,threadsPerBlock>>>(cuda_data,
              cuda_centroids,
              cuda_tmp_centroidCount,
              clusters);

              checkCuda(cudaGetLastError());
              cudaDeviceSynchronize();
              checkCuda(cudaMemcpyFromSymbol(&delta,cuda_delta,sizeof(unsigned int)));
              cudaDeviceSynchronize();

            }
            while(delta > 0);

            // copy output
            checkCuda(cudaMemcpy(label,cuda_membership, data_size *sizeof(int) , cudaMemcpyDeviceToHost));
            checkCuda(cudaMemcpy(centroids,cuda_centroids, clusters *sizeof(sequence_t) , cudaMemcpyDeviceToHost));
            cudaDeviceSynchronize();

            // free cuda memory
            checkCuda(cudaFree(cuda_data));
            checkCuda(cudaFree(cuda_centroids));
            checkCuda(cudaFree(cuda_membership));
            checkCuda(cudaFree(cuda_tmp_centroidCount));
            checkCuda(cudaDeviceReset());

            kmodes_result_t result = {
              label,
              centroids
            };
            return result;
          }
