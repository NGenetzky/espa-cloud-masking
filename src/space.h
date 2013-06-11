#ifndef SPACE_H
#define SPACE_H

#include <math.h>
#include "bool.h"

/* Constants */
#define NPROJ_PARAM (15)

typedef enum {SPACE_NOT_ISIN, SPACE_ISIN_NEST_1, SPACE_ISIN_NEST_2, 
              SPACE_ISIN_NEST_4} Space_isin_t;
#define SPACE_MAX_NEST (4)
#define STR_SIZE 1024

/* Integer image coordinates data structure */
typedef struct {
  int l;                /* line number */
  int s;                /* sample number */
} Img_coord_int_t;

/* Structure to store map projection coordinates */
typedef struct {
  double x;             /* Map projection X coordinate (meters) */
  double y;             /* Map projection Y coordinate (meters) */
  bool is_fill;         /* Flag to indicate whether the point is a fill value;
                           'true' = fill; 'false' = not fill */
} Map_coord_t;

/* Structure to store the space definition */
typedef struct {
  int proj_num;         /* GCTP map projection number */
  double proj_param[NPROJ_PARAM]; /* GCTP map projection parameters */
  float pixel_size;     /* Pixel size (meters) */
  Map_coord_t ul_corner;  /* Map projection coordinates of the upper left 
                             corner of the pixel in the upper left corner 
                             of the image */
  bool ul_corner_set;   /* Flag to indicate whether the upper left corner
                           has been set; 'true' = set; 'false' = not set */
  Img_coord_int_t img_size;  /* Image size (lines, samples) */
  int zone;             /* GCTP zone number */
  int sphere;           /* GCTP sphere number */
  bool zone_set;        /* Flag to indicate whether the zone has been set;
                           'true' = set; 'false' = not set */
  Space_isin_t isin_type;  /* Flag to indicate whether the projection is ISIN,
                              and if it is, the ISIN nesting */
  double orientation_angle;  /* Orientation of the image with respect to 
                                map north (radians).  A positive angle causes 
                                the image to be rotated counter-clockwise. */
} Space_def_t;

/* Prototypes */
int get_space_def_hdf
(
    Space_def_t *this,   /* I/O: spatial information structure which will be
                                 populated from the metadata */
    char *file_name,     /* I: name of the HDF file to read */
    char *grid_name      /* I: name of the grid to read metadata from */
);

int put_space_def_hdf
(
    Space_def_t *this,     /* I: space definition structure */
    char *file_name,       /* I: HDF file to write attributes to */
    int nsds,              /* I: number of SDS to write */
    char *sds_names[],     /* I: array of SDS names to write */
    int *sds_types,        /* I: array of types for each SDS */
    char *grid_name        /* I: name of the grid to write the SDSs to */
);

#endif
