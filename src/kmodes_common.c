#include "sequence.h"
#include "stdio.h"
#include "global.h"

__OFFLOAD__MODIFIER__ void logDistanceFromCluster(sequence_t sequence, int cluster, int distance) {
    printf("Distance from cluster %d\n", cluster);
    print_sequence(sequence);
    printf("\n");
    printf("Distance =: %d\n\n",distance);
}

__OFFLOAD__MODIFIER__ void logNearestDistance(int nearest, int min_distance) {
  #if DEBUG
  printf("Shortest distance is cluster %d\n",nearest);
  printf("\n");
  printf("Distance = %d\n\n",min_distance);
  #endif
}

__OFFLOAD__MODIFIER__ void logDistanceSequence(sequence_t sequence) {
  #if DEBUG
  printf("Calculating distance for sequence: ");
  print_sequence(sequence);
  printf("\n\n");
  #endif
}

__OFFLOAD__MODIFIER__ unsigned int maskForMode(unsigned int x,unsigned int y,unsigned int z,unsigned int w ) {
  unsigned int max = x > y ? x : y;
  max = z > max ? z : max;
  max = w > max ? w : max;
  unsigned int mask = 0;

  if (max == x){
    mask |= 0b0001;
  }
  if (max == y){
    mask |= 0b0010;
  }
  if (max == z){
    mask |= 0b0100;
  }
  if (max == w){
    mask |= 0b1000;
  }
  return mask;
}
