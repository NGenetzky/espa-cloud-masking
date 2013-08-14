#ifndef INPUT_H
#define INPUT_H

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "myhdf.h"
#include "const.h"
#include "date.h"
#include "error.h"
#include "mystring.h"
#include "cfmask.h"
#include "space.h"

/* Structure for bounding geographic coords */
typedef struct {
  double min_lon;  /* Geodetic longitude coordinate (degrees) */ 
  double min_lat;  /* Geodetic latitude coordinate (degrees) */ 
  double max_lon;  /* Geodetic longitude coordinate (degrees) */ 
  double max_lat;  /* Geodetic latitude coordinate (degrees) */ 
  bool is_fill;    /* Flag to indicate whether the point is a fill value; */
} Geo_bounds_t;

/* Structure for lat/long coordinates */
typedef struct {
  double lon;           /* Geodetic longitude coordinate (degrees) */ 
  double lat;           /* Geodetic latitude coordinate (degrees) */ 
  bool is_fill;         /* Flag to indicate whether the point is a fill value;
                           'true' = fill; 'false' = not fill */
} Geo_coord_t;

/* Structure for the metadata */
typedef struct {
  char provider[MAX_STR_LEN];  /* Data provider type */
  char sat[MAX_STR_LEN];       /* Satellite */
  char inst[MAX_STR_LEN];      /* Instrument */
  Date_t acq_date;             /* Acqsition date/time (scene center) */
  Date_t prod_date;            /* Production date (must be available for ETM) */
  float sun_zen;               /* Solar zenith angle (radians; scene center) */
  float sun_az;                /* Solar azimuth angle (radians; scene center) */
  char wrs_sys[MAX_STR_LEN];   /* WRS system */
  char unit_ref[MAX_STR_LEN];  
  char therm_unit_ref[MAX_STR_LEN];  
  int path;                    /* WRS path number */
  int row;                     /* WRS row number */
  int fill;                    /* Fill value for image data */
  int zone;
  float valid_range_ref[2];
  int satu_value_ref[NBAND_REFL_MAX]; 
  int satu_value_max[NBAND_REFL_MAX]; 
  float scale_factor_ref; 
  float add_offset_ref; 
  float scale_factor_err_ref; 
  float add_offset_err_ref; 
  float calibrated_nt_ref; 
  float therm_valid_range_ref[2];
  float therm_satu_value_ref; 
  int therm_satu_value_max; 
  float therm_scale_factor_ref; 
  float therm_add_offset_ref; 
  float therm_scale_factor_err_ref; 
  float therm_add_offset_err_ref; 
  float therm_calibrated_nt_ref; 
  float ul_lat;
  float ul_lon;
  int band[NBAND_REFL_MAX];    /* Band numbers */
  float gain[NBAND_REFL_MAX];  /* Band gain (MSS and TM only) */
  float bias[NBAND_REFL_MAX]; 
  float gain_th;           /* Thermal band gain (MSS and TM only) */
  float bias_th;           /* Thermal band bias (MSS and TM only) */
  Geo_coord_t ul_corner;   /* UL lat/long corner coord */
  Geo_coord_t lr_corner;   /* LR lat/long corner coord */
  Geo_bounds_t bounds;     /* Geographic bounding coordinates */
} Input_meta_t;

/* Structure for the 'input' data type */
typedef struct {
  char *lndth_name;        /* Input surface reflecetance image file name */
  char *lndcal_name;       /* Input TOA reflectance image file name */
  bool open;               /* Open file flag; open = true */
  Input_meta_t meta;       /* Input metadata */
  int nband;               /* Number of input image bands */
  Img_coord_int_t size;    /* Input file size */
  Img_coord_int_t toa_size;   /* Input file size */
  int32 sds_th_file_id;       /* SDS file id */
  int32 sds_cal_file_id;      /* SDS file id */
  Myhdf_sds_t sds[NBAND_REFL_MAX];
                           /* SDS data structures for TOA image data */
  int16 *buf[NBAND_REFL_MAX];
                           /* Input data buffer (one line of image data) */
  Myhdf_sds_t therm_sds;   /* SDS data structure for thermal image data */
  int16 *therm_buf;        /* Input data buffer (one line of thermal data) */
  float dsun_doy[366];
} Input_t;

typedef enum
{
    HDF_FILE = 0,
    BINARY_FILE
}File_type;

/* Prototypes */
Input_t *OpenInput(char *lndth_name, char *lndcal_name);
bool GetInputLine(Input_t *this, int iband, int iline);
bool GetInputQALine(Input_t *this, int iband, int iline);
bool GetInputThermLine(Input_t *this, int iline);
bool CloseInput(Input_t *this);
bool FreeInput(Input_t *this);
bool GetInputMeta(Input_t *this);
bool GetInputMeta2(Input_t *this);

bool potential_cloud_shadow_snow_mask
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
    unsigned char **final_mask, /*I/O: final combined pixel mask */
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
    unsigned char **final_mask, /*I/O: final combined pixel mask */
    bool verbose                /*I: value to indicate if intermediate messages 
                                     be printed */      
);

int write_envi_hdr
(
    char *hdr_file,        /* I: name of header file to be generated */
    File_type ftype,       /* I: HDF or Binary header is needed */
    Input_t *input,        /* I: input structure for cfmask products */
    Space_def_t *space_def /* I: spatial definition information */
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
    char **toa_infile,     /* O: address of input TOA filename */
    float *cloud_prob,     /* O: cloud_probability input */
    int *cldpix,           /* O: cloud_pixel buffer used for image dilate */
    int *sdpix,            /* O: shadow_pixel buffer used for image dilate  */
    bool *write_binary,    /* O: write raw binary flag */
    bool *no_hdf_output,   /* O: No HDF4 output file flag */
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
