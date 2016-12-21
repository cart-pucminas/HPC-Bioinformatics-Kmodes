#!/bin/sh
#PBS -S /bin/sh
#PBS -N Drosophila
#PBS -l nodes=9:cpp=4
#PBS -j oe
cd $PBS_O_WORKDIR

#mpiexec ./mpi-means/mpi-means ./input_mpi/gallus/cluster 12 9
#mpiexec ./mpi-means/mpi-means ./input_mpi/Rattusnovergicus/cluster 28 27
#mpiexec ./mpi-means/mpi-means ./input_mpi/Musmusculus/cluster 243 234
#mpiexec ./mpi-means/mpi-means ./input_mpi/celegans/cluster 7887 7884
#mpiexec ./mpi-means/mpi-means ./input_mpi/athaliana/cluster 21096 21897
mpiexec ./mpi-means/mpi-means ./input_mpi/Drosophila/cluster 17804 17802 
#mpiexec ./mpi-means/mpi-means ./input_mpi/HomoSapiens/cluster 14261 14256

