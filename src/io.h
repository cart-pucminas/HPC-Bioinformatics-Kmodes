#ifndef _IO_
#define _IO_

#include "global.h"
#include "sequence.h"
#include "kmodes.h"
 kmodes_input_t read_data(const char *);
 void write_nearest_objects(const char *,kmodes_input_t, kmodes_result_t);

#endif
