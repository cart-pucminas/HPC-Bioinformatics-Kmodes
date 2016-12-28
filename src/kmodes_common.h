#ifndef _KMODES_COOMON_
#define _KMODES_COOMON_

#include "sequence.h"
#include "kmodes_common.h"

__OFFLOAD__MODIFIER__ void logDistanceFromCluster(sequence_t sequence, int cluster, int distance);
__OFFLOAD__MODIFIER__ void logNearestDistance(int nearest, int min_distance);
__OFFLOAD__MODIFIER__ void logDistanceSequence(sequence_t sequence);
__OFFLOAD__MODIFIER__ unsigned int maskForMode(unsigned int x,unsigned int y,unsigned int z,unsigned int w);

#endif
