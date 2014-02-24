#ifndef INPUT_H
#define INPUT_H

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "date.h"
#include "cfmask.h"

/* Structure for the metadata */
typedef struct {
  char sat[MAX_STR_LEN];       /* Satellite */
  Date_t acq_date;             /* Acqsition date/time (scene center) */
  float sun_zen;               /* Solar zenith angle (degrees; scene center) */
  float sun_az;                /* Solar azimuth angle (degrees; scene center) */
  int fill;                    /* Fill value for image data */
  float pixel_size[2];         /* pixel size (x,y) */
  int satu_value_ref[NBAND_REFL_MAX];  /* saturation value of TOA products */
  int satu_value_max[NBAND_REFL_MAX];  /* maximum TOA value */
  int therm_satu_value_ref;    /* saturation value of thermal product */
  int therm_satu_value_max;    /* maximum bt value (degrees Celsius) */
  float gain[NBAND_REFL_MAX];  /* Band gain */
  float bias[NBAND_REFL_MAX];  /* Band bias */
  float gain_th;               /* Thermal band gain */
  float bias_th;               /* Thermal band bias */
  float therm_scale_fact;      /* Scale factor for the thermal band */
  Geo_coord_t ul_corner;       /* UL lat/long corner coord */
  Geo_coord_t lr_corner;       /* LR lat/long corner coord */
} Input_meta_t;

/* Structure for the 'input' data type */
typedef struct {
  Input_meta_t meta;       /* Input metadata */
  int nband;               /* Number of input TOA reflectance bands */
  Img_coord_int_t size;    /* Input file size */
  char *file_name[NBAND_REFL_MAX];  /* Name of the input TOA image files */
  char *file_name_therm;   /* Name of the input thermal file */
  FILE *fp_bin[NBAND_REFL_MAX];  /* File pointer for TOA refl binary files */
  bool open[NBAND_REFL_MAX]; /* Flag to indicate whether the specific input TOA
                                reflectance file is open for access;
                                'true' = open, 'false' = not open */
  int16 *buf[NBAND_REFL_MAX]; /* Input data buffer (one line of image data) */
  FILE *fp_bin_therm;      /* File pointer for thermal binary file */
  bool open_therm;         /* Flag to indicate whether the input thermal
                              file is open for access */
  int16 *therm_buf;        /* Input data buffer (one line of thermal data) */
  float dsun_doy[366];     /* Array of earth/sun distances for each DOY; read
                              from the EarthSunDistance.txt file */
} Input_t;

/* Prototypes */
Input_t *OpenInput(Espa_internal_meta_t *metadata);
bool GetInputLine(Input_t *this, int iband, int iline);
bool GetInputThermLine(Input_t *this, int iline);
bool CloseInput(Input_t *this);
bool FreeInput(Input_t *this);
bool GetXMLInput(Input_t *this, Espa_internal_meta_t *metadata);

int potential_cloud_shadow_snow_mask
(
    Input_t *input,             /*I: input structure */
    float cloud_prob_threshold, /*I: cloud probability threshold */
    float *ptm,                 /*O: percent of clear-sky pixels */
    float *t_templ,             /*O: percentile of low background temperature */
    float *t_temph,             /*O: percentile of high background temperature */
    unsigned char **cloud_mask, /*I/O: cloud pixel mask */
    unsigned char **shadow_mask,/*I/O: cloud shadow pixel mask */
    unsigned char **snow_mask,  /*I/O: snow pixel mask */
    unsigned char **water_mask, /*I/O: water pixel mask */
    bool verbose                /*I: value to indicate if intermediate messages be 
                                     printed */
);

int object_cloud_shadow_match
(
    Input_t *input,             /*I: input structure */
    float ptm,                  /*I: percent of clear-sky pixels */
    float t_templ,              /*I: percentile of low background temperature */
    float t_temph,              /*I: percentile of high background temperature */
    int cldpix,                 /*I: cloud buffer size */
    int sdpix,                  /*I: shadow buffer size */
    unsigned char **cloud_mask, /*I/O: cloud pixel mask */
    unsigned char **shadow_mask,/*I/O: cloud shadow pixel mask */
    unsigned char **snow_mask,  /*I/O: snow pixel mask */
    unsigned char **water_mask, /*I/O: water pixel mask */
    bool verbose                /*I: value to indicate if intermediate messages 
                                     be printed */      
);

void split_filename 
(
    const char *filename,       /* I: Name of file to split */
    char *directory,            /* O: Directory portion of file name */
    char *scene_name,           /* O: Scene name portion of the file name */
    char *extension             /* O: Extension portion of the file name */
);

int prctile
(
    int16 *array,    /*I: input data pointer */ 
    int nums,        /*I: number of input data array */  
    int16 min,       /*I: minimum value in the input data array */  
    int16 max,       /*I: maximum value in the input data array  */  
    float prct,      /*I: percentage threshold */ 
    float *result    /*O: percentile calculated */ 
); 

int prctile2
(
    float *array,     /*I: input data pointer */ 
    int nums,         /*I: number of input data array */   
    float min,        /*I: minimum value in the input data array */ 
    float max,        /*I: maximum value in the input data array  */ 
    float prct,       /*I: percentage threshold */ 
    float *result     /*O: percentile calculated */ 
); 

int get_args
(
    int argc,              /* I: number of cmd-line args */
    char *argv[],          /* I: string of cmd-line args */
    char **xml_infile,     /* O: address of input XML filename */
    float *cloud_prob,     /* O: cloud_probability input */
    int *cldpix,           /* O: cloud_pixel buffer used for image dilate */
    int *sdpix,            /* O: shadow_pixel buffer used for image dilate  */
    bool *verbose          /* O: verbose flag */
);

void usage ();

void error_handler
(
    bool error_flag,  /* I: true for errors, false for warnings */
    char *module,     /* I: calling module name */
    char *errmsg      /* I: error message to be printed, without ending EOL */
);

#endif
