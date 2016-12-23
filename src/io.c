#define _GNU_SOURCE

#include "io.h"
#include "sequence.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>


/**
 * An plataform independent version of the strtoul or strtoull
 * using base 2.
 * @param binary_string an binary string per example "100010001000"
 * @return an 64bit interger represented by the string
 */
uint64_t binary_str_to_64_integer(char *binary_string) {
  char* start = &binary_string[0];
  uint64_t total = 0;
  while (*start)
  {
    total <<= 1;
    if (*start++ == '1') total^=1;
  }
  return total;
}


/**
 * Append a characterist using k-modes format to the buffer
 * Buffer the null terminate string that will have the characterist append
 * @param characterist char '1' or '0'
 * @return A null terminate string with the append characterist in kmodes format
 *         0010 if characterist is '0' and 0001 if charecterist is '1'
 */
void appendCharacterist(char *buffer, char characterist) {
  if (characterist == '1') {
    strcat(buffer, "0001");
  } else {
    strcat(buffer, "0010");
  }
}

/**
 * Parse an binary string (100010000100) into a sequence
 * @param binary_string the string to be parsed
 * @return the parsed sequence
 */
sequence_t parse_sequence(const char * binary_string) {
  char subbuff[65];

  memset(subbuff, '\0', 65);
  memcpy(subbuff, &binary_string[0], 64);
  uint64_t x = binary_str_to_64_integer(subbuff);

  memset(subbuff, '\0', 65);
  memcpy(subbuff, &binary_string[64], 64);
  uint64_t y =  binary_str_to_64_integer(subbuff);

  memset(subbuff, '\0', 65);
  memcpy(subbuff, &binary_string[128], strlen(binary_string) - 128);

  subbuff[strcspn(subbuff, "\n")] = 0; // strip out \n

  char characteristic3 = subbuff[strlen(subbuff)-1];
  char characteristic2 = subbuff[strlen(subbuff)-1];
  char characteristic1 = subbuff[strlen(subbuff)-1];

  subbuff[strlen(subbuff)-1] = 0;
  subbuff[strlen(subbuff)-1] = 0;
  subbuff[strlen(subbuff)-1] = 0;

  appendCharacterist(subbuff, characteristic1);
  appendCharacterist(subbuff, characteristic2);
  appendCharacterist(subbuff, characteristic3);

  uint64_t z =  binary_str_to_64_integer(subbuff);

  sequence_t seq = {  x, y , z };
  return seq;
}

kmodes_input_t read_data(const char *file) {
  size_t line_size = 255;
  char line[line_size];
  FILE *in = fopen(file, "r");

  if(!in) {
    printf("Invalid file!\n");
    exit(0);
  } else {
    safe_print("Reading file %s\n", file);
  }

  //Count lines
  size_t data_size = 0;
  rewind(in);

  while(fgets(line, line_size, in) != NULL) {
    if(strlen(line) > 1) {
      data_size++;
    }
  }

  safe_print("Number of lines = %zu\n", data_size);

  //Read objects
  sequence_t *data = (sequence_t*)calloc(data_size, sizeof(sequence_t));
  rewind(in);

  size_t current_line = 0;
  while(fgets(line, line_size, in) != NULL) {
    if(strlen(line) > 1) {
      data[current_line] = parse_sequence(line);
      current_line++;
    } else if (current_line + 1 < data_size) {
      printf("Error parsing line: %zu\n", current_line);
    }
  }

  safe_print("Finish read input file\n");
  fclose(in);
  kmodes_input_t input = { data, data_size, 0 };
  return input;
}

void write_nearest_objects(const char *file, kmodes_input_t input, kmodes_result_t result) {

  #if USE_MPI
  if (mpi_rank != 0) {
    return;
  }
  #endif


  FILE *out = fopen(file,"w");
  int nearest;
  float distance,min_distance;

  if(!out) {
    printf("Invalid file!\n");
    exit(0);
  }

  //For each centroid
  for(size_t i = 0; i < input.number_of_clusters; i++) {
    nearest = -1;
    min_distance = FLT_MAX;

    //Find the nearest object
    for(size_t j = 0;j < input.data_size; j++) {
      if(result.labels[j] != (int)i) {
        continue;
      }

      distance = dist_sequence (input.data[j], result.centroids[i]);

      if(distance < min_distance) {
        nearest = j;
        min_distance = distance;
      }
    }

    if(nearest == -1) {
      nearest = i * input.data_size / input.number_of_clusters;
    }
    char *sequence_str = to_human_readble_string(input.data[nearest]);
    fprintf(out,"%s ", sequence_str);
    fprintf(out,"\n");
  }

  fclose(out);
}
