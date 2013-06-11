#ifndef HDF_SAMPLE_CODE_H
#define HDF_SAMPLE_CODE_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include "bool.h"
#include "const.h"
#include "date.h"
#include "error.h"
#include "mystring.h"
#include "myhdf.h"

/* Type definitions */
typedef enum {FAILURE = 0, SUCCESS = 1} Status_t;

/* There are currently a maximum of 6 reflective bands in the output surface
   reflectance product */
#define NBAND_REFL_MAX 6

/* QA bands - fill_QA, DDV_QA, cloud_QA, cloud_shadow_QA, snow_QA,
   land_water_QA, adjacent_cloud_QA */
typedef enum {
  FILL,
  DDV,
  CLOUD,
  CLOUD_SHADOW,
  SNOW,
  LAND_WATER,
  ADJ_CLOUD,
  NUM_QA_BAND
} QA_Band_t;

typedef struct{
    int proj_code;      /* Projection code */
    int zone;           /* Projection zone number - only has meaning for
                           projections like UTM and stateplane */
    int units;          /* Units of coordinates */
    int spheroid;       /* Spheroid code for the projection */
    double parameters[MAX_STR_LEN];
                        /* Array of projection parameters */
} IAS_PROJECTION;

#endif
