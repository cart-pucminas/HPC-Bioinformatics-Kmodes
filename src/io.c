#define _GNU_SOURCE

#include "io.h"
#include "sequence.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>

unsigned char baseForBinary(unsigned char b){
  if (b == 1)
  return 'a';
  if (b == 2)
  return 'c';
  if (b == 4)
  return 'g';
  if (b == 8)
  return 't';
  assert(1);
  return '\0';
}
void printBits(FILE *out,void const * const ptr)
{
  unsigned char *b = (unsigned char*) ptr;
  size_t size = sizeof(uint64_t);
  int i;
  unsigned char byte;
  for (i=size-1;i>=0;i--)
  {
    byte = b[i] >> 4;
    fprintf(out,"%c", baseForBinary(byte));
    byte = b[i] &= 0xf;
    fprintf(out,"%c", baseForBinary(byte));
  }
  fputs("",out);
}

void printSpecialCase(FILE *out,uint64_t *ptr)
{
  unsigned char *b = (unsigned char*) ptr;
  size_t size = sizeof(unsigned int); // only half of the bits will be used on Z
  int i;

  // remove caracteristic bits
  uint64_t threeLast = ((*ptr) & 0xFFFUL);
  *ptr >>= 12;

  unsigned char byte;
  for (i=size-1;i>=0;i--)
  {
    byte = b[i] >> 4;
    fprintf(out,"%c", baseForBinary(byte));
    byte = b[i] &= 0xf;
    fprintf(out,"%c", baseForBinary(byte));
  }
  fputs(" ",out);
  byte = threeLast >> 8;
  if (byte == 1) {
    fprintf(out,"%d",1);
  } else if (byte == 2) {
    fprintf(out,"%d",0);
  }
  byte = (threeLast >> 4) & 0xF;
  if (byte == 1) {
    fprintf(out,"%d",1);
  } else if (byte == 2) {
    fprintf(out,"%d",0);
  }
  byte = threeLast & 0xF;
  if (byte == 1) {
    fprintf(out,"%d",1);
  } else if (byte == 2){
    fprintf(out,"%d",0);
  }
}

uint64_t strtoul64(char *s) {
  char* start = &s[0];
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

kmodes_input_t read_data(const char *file) {
  size_t line_size = 255;
  char line[line_size];
  FILE *in = fopen(file, "r");

  if(!in) {
    printf("Invalid file!\n");
    exit(0);
  } else {
    printf("Reading file %s\n", file);
  }

  //Count lines
  size_t data_size = 0;
  rewind(in);

  while(fgets(&line,&line_size,in) != NULL) {
    if(strlen(line) > 1) {
      data_size++;
    }
  }

  printf("Number of lines = %lu\n", data_size);

  //Read objects
  sequence_t *data = (sequence_t*)calloc(data_size, sizeof(sequence_t));
  rewind(in);

  size_t current_line = 0;
  while(fgets(&line,&line_size,in) != NULL) {
    if(strlen(line) > 1) {

      char subbuff[65];

      memset(subbuff, '\0', 65);
      memcpy(subbuff, &line[0], 64);
      uint64_t x = strtoul64(subbuff);

      memset(subbuff, '\0', 65);
      memcpy(subbuff, &line[64], 64);
      uint64_t y =  strtoul64(subbuff);

      memset(subbuff, '\0', 65);
      memcpy(subbuff, &line[128], strlen(line) - 128);

      subbuff[strcspn(subbuff, "\n")] = 0; // strip out \n

      char charecterist3 = subbuff[strlen(subbuff)-1];
      char charecterist2 = subbuff[strlen(subbuff)-1];
      char charecterist1 = subbuff[strlen(subbuff)-1];

      subbuff[strlen(subbuff)-1] = 0;
      subbuff[strlen(subbuff)-1] = 0;
      subbuff[strlen(subbuff)-1] = 0;

      appendCharacterist(subbuff, charecterist1);
      appendCharacterist(subbuff, charecterist2);
      appendCharacterist(subbuff, charecterist3);

      uint64_t z =  strtoul64(subbuff);

      sequence_t seq = {  x, y , z };
      data[current_line] = seq;
      current_line++;
    } else if (current_line + 1 < data_size) {
      printf("Error parsing line: %zu\n", current_line);
    }
  }

  printf("Finish read input file\n");
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

    printBits(out,&input.data[nearest].x);
    fprintf(out," ");
    printBits(out,&input.data[nearest].y);
    fprintf(out," ");
    printSpecialCase(out,&input.data[nearest].z);
    fprintf(out," ");
    fprintf(out,"\n");
  }

  fclose(out);
}
