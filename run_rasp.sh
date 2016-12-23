#!/bin/sh


set -x # print all commands

for i in 2 3 4 5 6 7 8 9 10 11 12
do
  #clean up
  ssh cart-pi$i rm -rf /home/guilherme.castro/CudaPredTIS
  # copy binary
  ssh cart-pi$i mkdir -p /home/guilherme.castro/CudaPredTIS/bin
  scp ./bin/kmodes-mpi cart-pi$i:/home/guilherme.castro/CudaPredTIS/bin/kmodes-mpi
  #create input dir
  ssh cart-pi$i mkdir -p /home/guilherme.castro/CudaPredTIS/input/athaliana
  ssh cart-pi$i mkdir -p /home/guilherme.castro/CudaPredTIS/input/celegans
  ssh cart-pi$i mkdir -p /home/guilherme.castro/CudaPredTIS/input/Drosophila
  ssh cart-pi$i mkdir -p /home/guilherme.castro/CudaPredTIS/input/gallus
  ssh cart-pi$i mkdir -p /home/guilherme.castro/CudaPredTIS/input/HomoSapiens
  ssh cart-pi$i mkdir -p /home/guilherme.castro/CudaPredTIS/input/Musmusculus
  ssh cart-pi$i mkdir -p /home/guilherme.castro/CudaPredTIS/input/Rattusnovergicus
  #copy input
  scp ./input/gallus/clusters.tar.gz cart-pi$i:/home/guilherme.castro/CudaPredTIS/input/gallus/clusters.tar.gz
  # scp ./input/Rattusnovergicus/clusters.tar.gz cart-pi$i:/home/guilherme.castro/CudaPredTIS/input/Rattusnovergicus/clusters.tar.gz
  # scp ./input/Musmusculus/clusters.tar.gz cart-pi$i:/home/guilherme.castro/CudaPredTIS/input/Musmusculus/clusters.tar.gz
  # scp ./input/athaliana/clusters.tar.gz cart-pi$i:/home/guilherme.castro/CudaPredTIS/input/athaliana/clusters.tar.gz
  # scp ./input/celegans/clusters.tar.gz cart-pi$i:/home/guilherme.castro/CudaPredTIS/input/celegans/clusters.tar.gz
  # scp ./input/Drosophila/clusters.tar.gz cart-pi$i:/home/guilherme.castro/CudaPredTIS/input/Drosophila/clusters.tar.gz
  # scp ./input/HomoSapiens/clusters.tar.gz cart-pi$i:/home/guilherme.castro/CudaPredTIS/input/HomoSapiens/clusters.tar.gz
  # extract input
  ssh cart-pi$i tar -zxkvf /home/guilherme.castro/CudaPredTIS/input/gallus/clusters.tar.gz -C /home/guilherme.castro/CudaPredTIS/input/gallus/
  # ssh tar -zxkvf cart-pi$i /home/guilherme.castro/CudaPredTIS/input/Rattusnovergicus/clusters.tar.gz -C /home/guilherme.castro/CudaPredTIS/input/Rattusnovergicus/
  # ssh tar -zxkvf cart-pi$i /home/guilherme.castro/CudaPredTIS/input/Musmusculus/clusters.tar.gz -C /home/guilherme.castro/CudaPredTIS/input/Musmusculus/
  # ssh tar -zxkvf cart-pi$i /home/guilherme.castro/CudaPredTIS/input/athaliana/clusters.tar.gz -C /home/guilherme.castro/CudaPredTIS/input/athaliana/
  # ssh tar -zxkvf cart-pi$i /home/guilherme.castro/CudaPredTIS/input/celegans/clusters.tar.gz -C /home/guilherme.castro/CudaPredTIS/input/celegans/
  # ssh tar -zxkvf cart-pi$i /home/guilherme.castro/CudaPredTIS/input/Drosophila/clusters.tar.gz -C /home/guilherme.castro/CudaPredTIS/input/Drosophila/
  # ssh tar -zxkvf cart-pi$i /home/guilherme.castro/CudaPredTIS/input/HomoSapiens/clusters.tar.gz -C /home/guilherme.castro/CudaPredTIS/input/HomoSapiens/

done

mpirun -hostfile host_file  -np 12 ./bin/kmodes-mpi ./input/gallus/cluster 12 9 > gallus.log
