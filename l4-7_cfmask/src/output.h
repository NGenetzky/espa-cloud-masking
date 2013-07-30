#ifndef OUTPUT_H
#define OUTPUT_H

#include "cfmask.h"
#include "input.h"

#define NUM_OUT_SDS 1

/* Structure for the 'output' data type */

typedef struct {
  char *file_name;      /* Output file name */
  bool open;            /* Flag to indicate whether output file is open 
                           for access; 'true' = open, 'false' = not open */
  Img_coord_int_t size; /* Output image size */
  int nband;            /* Number of output image bands */
  int32 sds_file_id;    /* SDS file id */
  Myhdf_sds_t sds[NUM_OUT_SDS]; /* SDS data structures for image data */
} Output_t;

/* Prototypes */

bool CreateOutput(char *file_name);
Output_t *OpenOutput(char *file_name, char sds_names[MAX_STR_LEN],
   Img_coord_int_t *size);
bool PutOutput(Output_t *this, unsigned char **final_mask);
bool CloseOutput(Output_t *this);
bool FreeOutput(Output_t *this);
bool PutMetadata(Output_t *this, Input_t *input);

#endif
