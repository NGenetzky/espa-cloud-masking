/*****************************************************************************
!File: input.c
*****************************************************************************/

#include "input.h"

#define SDS_PREFIX ("band")

#define INPUT_PROVIDER ("DataProvider")
#define INPUT_SAT ("Satellite")
#define INPUT_INST ("Instrument")
#define INPUT_ACQ_DATE ("AcquisitionDate")
#define INPUT_PROD_DATE ("Level1ProductionDate")
#define INPUT_SUN_ZEN ("SolarZenith")
#define INPUT_SUN_AZ ("SolarAzimuth")
#define INPUT_WRS_SYS ("WRS_System")
#define INPUT_WRS_PATH ("WRS_Path")
#define INPUT_WRS_ROW ("WRS_Row")
#define INPUT_NBAND ("NumberOfBands")
#define INPUT_BANDS ("BandNumbers")
#define INPUT_FILL_VALUE ("_FillValue")
#define INPUT_WEST_BOUND  ("WestBoundingCoordinate")
#define INPUT_EAST_BOUND  ("EastBoundingCoordinate")
#define INPUT_NORTH_BOUND ("NorthBoundingCoordinate")
#define INPUT_SOUTH_BOUND ("SouthBoundingCoordinate")

#define INPUT_UNITS            ("units")
#define INPUT_VALID_RANGE      ("valid_range")
#define INPUT_SATU_VALUE       ("_SaturateValue")
#define INPUT_SCALE_FACTOR     ("scale_factor")
#define INPUT_ADD_OFFSET       ("add_offset")
#define INPUT_SCALE_FACTOR_ERR ("scale_factor_err")
#define INPUT_ADD_OFFSET_ERR   ("add_offset_err")
#define INPUT_CALIBRATED_NT    ("calibrated_nt")

#define N_LSAT_WRS1_ROWS  (251)
#define N_LSAT_WRS1_PATHS (233)
#define N_LSAT_WRS2_ROWS  (248)
#define N_LSAT_WRS2_PATHS (233)

#if 0
typedef enum {
  HEADER_NULL = -1,
  HEADER_START = 0,
  HEADER_FILE_TYPE,
  HEADER_PROIVDER,
  HEADER_SAT,
  HEADER_INST,
  HEADER_ACQ_DATE,
  HEADER_ACQ_TIME,
  HEADER_PROD_DATE,
  HEADER_SUN_ZEN,
  HEADER_SUN_AZ,
  HEADER_WRS_SYS,
  HEADER_WRS_PATH,
  HEADER_WRS_ROW,
  HEADER_NBAND,
  HEADER_BANDS,
  HEADER_GAIN_SET,
  HEADER_GAIN_SETTINGS_TH,
  HEADER_GAIN,
  HEADER_BIAS,
  HEADER_NSAMPLE,
  HEADER_NLINE,
  HEADER_FILES,
  HEADER_NBAND_TH,
  HEADER_BANDS_TH,
  HEADER_FILES_TH,
  HEADER_NSAMPLE_TH,
  HEADER_NLINE_TH,
  HEADER_GAIN_TH,
  HEADER_BIAS_TH,
  HEADER_END,
  HEADER_MAX
} Header_key_t;

Key_string_t Header_string[HEADER_MAX] = {
  {(int)HEADER_START,     "HEADER_FILE"},
  {(int)HEADER_FILE_TYPE, "FILE_TYPE"},
  {(int)HEADER_PROIVDER,  "DATA_PROVIDER"},
  {(int)HEADER_SAT,       "SATTELITE"},
  {(int)HEADER_INST,      "INSTRUMENT"},
  {(int)HEADER_ACQ_DATE,  "ACQUISITION_DATE"},
  {(int)HEADER_ACQ_TIME,  "ACQUISITION_TIME"},
  {(int)HEADER_PROD_DATE, "PRODUCTION_DATE"},
  {(int)HEADER_SUN_ZEN,   "SOLAR_ZENITH"},
  {(int)HEADER_SUN_AZ,    "SOLAR_AZIMUTH"},
  {(int)HEADER_WRS_SYS,   "WRS_SYSTEM"},
  {(int)HEADER_WRS_PATH,  "WRS_PATH"},
  {(int)HEADER_WRS_ROW,   "WRS_ROW"},
  {(int)HEADER_NBAND,     "NBAND"},
  {(int)HEADER_BANDS,     "BANDS"},
  {(int)HEADER_GAIN_SET,  "GAIN_SETTINGS"},
  {(int)HEADER_GAIN_SETTINGS_TH, "GAIN_SETTINGS_TH"},
  {(int)HEADER_GAIN,      "GAIN"},
  {(int)HEADER_BIAS,      "BIAS"},
  {(int)HEADER_NSAMPLE,   "NSAMPLE"},
  {(int)HEADER_NLINE,     "NLINE"},
  {(int)HEADER_FILES,     "FILE_NAMES"},
  {(int)HEADER_NBAND_TH,  "NBAND_TH"},
  {(int)HEADER_BANDS_TH,  "BANDS_TH"},
  {(int)HEADER_FILES_TH,  "FILE_NAMES_TH"},
  {(int)HEADER_NSAMPLE_TH,"NSAMPLE_TH"},
  {(int)HEADER_NLINE_TH,  "NLINE_TH"},
  {(int)HEADER_GAIN_TH,   "GAIN_TH"},
  {(int)HEADER_BIAS_TH,   "BIAS_TH"},
  {(int)HEADER_END,       "END"}
};

Key_string_t Input_type_string[INPUT_TYPE_MAX] = {
  {(int)INPUT_TYPE_BINARY,              "BINARY"},
  {(int)INPUT_TYPE_BINARY_2BYTE_BIG,    "BINARY_2BYTE_BIG"},
  {(int)INPUT_TYPE_BINARY_2BYTE_LITTLE, "BINARY_2BYTE_LITTLE"},
  {(int)INPUT_TYPE_GEOTIFF,             "GEOTIFF"}
};
#endif

char *trimwhitespace(char *str)
{
  char *end;

  /* Trim leading space */
  while(isspace(*str)) str++;

  if(*str == 0)  
    return str;

  /* Trim trailing space */
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  /* Write new null terminator */
  *(end+1) = 0;

  return str;
}

/******************************************************************************
MODULE:  dn_to_bt

PURPOSE:  Convert Digital Number to Brightness Temperature

RETURN: true on success
        false on error

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
3/15/2013   Song Guo         Original Development

NOTES: 
******************************************************************************/
int dn_to_bt(Input_t *input)
{
    float k1, k2;
    int dn = 255;

    if(strcmp(input->meta.sat, "LANDSAT_7") == 0)
    {
       k1 = 666.09;
       k2 = 1282.71;
    }
    else if(strcmp(input->meta.sat, "LANDSAT_5") == 0)
    {
       k1 = 607.76;
       k2 = 1260.56;
    }
    else if(strcmp(input->meta.sat, "LANDSAT_4") == 0)
    {
       k1 = 671.62;
       k2 = 1284.30;
    }
    else
    {
       printf("Unsupported satellite sensor\n");
       return false;
    }

    float temp;
    temp = (input->meta.gain_th * (float)dn) + 
            input->meta.bias_th;
    temp = k2 / log ((k1/temp)+1);
    input->meta.therm_satu_value_max  = (int) (100.0 * (temp - 273.15) + 0.5);    

    return true;
}

/******************************************************************************
MODULE:  dn_to_toa

PURPOSE: Convert Digital Number to TOA reflectance 

RETURN: true on success
        false on error 

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
3/15/2013   Song Guo         Original Development

NOTES: 
******************************************************************************/
int dn_to_toa(Input_t *input)
{
    float esun[6];
    int ib;
    int dn = 255;

    if(strcmp(input->meta.sat, "LANDSAT_7") == 0)
    {
     esun[0] = 1997.0;
     esun[1] = 1812.0;
     esun[2] = 1533.0;
     esun[3] = 1039.0;
     esun[4] = 230.8;
     esun[5] = 84.9;
    }
    else if(strcmp(input->meta.sat, "LANDSAT_5") == 0)
    {
     esun[0] = 1983.0;
     esun[1] = 1796.0;
     esun[2] = 1536.0;
     esun[3] = 1031.0;
     esun[4] = 220.0;
     esun[5] = 83.44;
    }
    else if(strcmp(input->meta.sat, "LANDSAT_4") == 0)
    {
     esun[0] = 1983.0;
     esun[1] = 1795.0;
     esun[2] = 1539.0;
     esun[3] = 1028.0;
     esun[4] = 219.8;
     esun[5] = 83.49;
    }
    else
    {
       printf("Unsupported satellite sensor\n");
       return false;
    }

    float temp;
    for (ib = 0; ib < NBAND_REFL_MAX; ib++)
    {
        temp = (input->meta.gain[ib] * (float)dn) + 
                input->meta.bias[ib];
        input->meta.satu_value_max[ib] = (int)((10000.0 * PI * temp * 
                input->dsun_doy[input->meta.acq_date.doy-1] * 
                input->dsun_doy[input->meta.acq_date.doy-1]) / 
                (esun[ib] * cos(input->meta.sun_zen * (PI/180.0))) + 0.5);
    }

    return true;
}


/******************************************************************************
MODULE:  getMeta

PURPOSE: Get needed metadata from the LEDAPS generated metadata file

RETURN: 0 on success
        -1 on error
HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
3/15/2013   Song Guo         

NOTES: 
******************************************************************************/
int getMeta(char meta_filename[], Input_t *this)
{
    /* vars used in parameter parsing */
    char  buffer[MAX_STR_LEN] = "\0";
    char  *label = NULL;
    char  *label2 = NULL;
    char  *tokenptr = NULL;
    char  *tokenptr2 = NULL;
    char  *seperator = "=";
    char  *seperator2 = ",";
    FILE *in;
    int ib;

    in=fopen(meta_filename, "r");
    if (in == NULL)
        return -1;

    /* process line by line */
    while(fgets(buffer, MAX_STR_LEN, in) != NULL) {

    char *s;
    s = strchr(buffer, '=');
    if (s != NULL)
    {
        /* get string token */
        tokenptr = strtok(buffer, seperator);
        label=trimwhitespace(tokenptr);
 
        if (strcmp(label,"UPPER_LEFT_CORNER") == 0)
        {
            tokenptr = trimwhitespace(strtok(NULL, seperator));
        }

        if (strcmp(label,"UPPER_LEFT_CORNER") == 0)
        {
            tokenptr2 = strtok(tokenptr, seperator2);
            label2=trimwhitespace(tokenptr2);
            tokenptr2 = trimwhitespace(strtok(NULL, seperator2));
            this->meta.ul_projection_x = atof(label2);
            this->meta.ul_projection_y = atof(tokenptr2);
        }

        if (strcmp(label,"PROJECTION_NUMBER") == 0)
        {
            tokenptr = trimwhitespace(strtok(NULL, seperator));
            this->meta.zone = atoi(tokenptr);
        }
        if (strcmp(label,"GAIN") == 0)
        {
            tokenptr = trimwhitespace(strtok(NULL, seperator));
        }

        if (strcmp(label,"GAIN") == 0)
        {
            tokenptr2 = strtok(tokenptr, seperator2);
            ib = 0;
            while(tokenptr2 != NULL)
            {
                this->meta.gain[ib] = atof(tokenptr2);
                tokenptr2 = strtok(NULL, seperator2);
                ib++;
            }
        }
        if (strcmp(label,"BIAS") == 0)
        {
            tokenptr = trimwhitespace(strtok(NULL, seperator));
        }

        if (strcmp(label,"BIAS") == 0)
        {
            tokenptr2 = strtok(tokenptr, seperator2);
            ib = 0;
            while(tokenptr2 != NULL)
            {
                this->meta.bias[ib] = atof(tokenptr2);
                tokenptr2 = strtok(NULL, seperator2);
                ib++;
            }
        }
        if (strcmp(label,"GAIN_TH") == 0)
        {
            tokenptr = trimwhitespace(strtok(NULL, seperator));
            this->meta.gain_th = atof(tokenptr);
        }
        if (strcmp(label,"BIAS_TH") == 0)
        {
            tokenptr = trimwhitespace(strtok(NULL, seperator));
            this->meta.bias_th = atof(tokenptr);
        }
      }
    }
    fclose(in);

    int i;  
    in = fopen("EarthSunDistance.txt", "r");
    for (i = 0; i < 366; i++)
        fscanf(in, "%f", &this->dsun_doy[i]);
    fclose(in);

    /* Calculate maximum TOA reflectance values and put them in metadata */
    dn_to_toa(this);
    /* Calculate maximum BT values and put them in metadata */
    dn_to_bt(this);

    return 0;
}


/******************************************************************************
!Description: 'OpenInput' sets up the 'input' data structure, opens the
 input file for read access, allocates space, and stores some of the metadata.
 
!Input Parameters:
 file_name      input file name

!Output Parameters:
 (returns)      populated 'input' data structure or NULL when an error occurs

!Team Unique Header:

!Design Notes:
******************************************************************************/
Input_t *OpenInput(char *lndth_name, char *lndcal_name, char *lndmeta_name)
{
  Input_t *this;
  Myhdf_attr_t attr;
  char *error_string = NULL;
  char sds_name[40];
  int ir;
  Myhdf_dim_t *dim[2];
  int ib;
  double dval[NBAND_REFL_MAX];
  int16 *buf = NULL;
  int status;

  /* Create the Input data structure */
  this = (Input_t *)malloc(sizeof(Input_t));
  if (this == NULL) 
    RETURN_ERROR("allocating Input data structure", "OpenInput", NULL);

  /* Populate the data structure */
  this->lndth_name = DupString(lndth_name);
  if (this->lndth_name == NULL) {
    free(this);
    RETURN_ERROR("duplicating file name", "OpenInput", NULL);
  }

  this->lndcal_name = DupString(lndcal_name);
  if (this->lndcal_name == NULL) {
    free(this);
    RETURN_ERROR("duplicating file name", "OpenInput", NULL);
  }

  /* Open file for SD access */
  this->sds_sr_file_id = SDstart((char *)lndth_name, DFACC_RDONLY);
  if (this->sds_sr_file_id == HDF_ERROR) {
    free(this->lndth_name);
    free(this->lndcal_name);
    free(this);  
    RETURN_ERROR("opening lndth input file", "OpenInput", NULL); 
  }
  /* Open file for SD access */
  this->sds_cal_file_id = SDstart((char *)lndcal_name, DFACC_RDONLY);
  if (this->sds_cal_file_id == HDF_ERROR) {
    free(this->lndth_name);
    free(this->lndcal_name);
    free(this);  
    RETURN_ERROR("opening lndcal input file", "OpenInput", NULL); 
  }
  this->open = true;

  /* Get the input metadata */
  if (!GetInputMeta(this)) {
    free(this->lndth_name);
    free(this->lndcal_name);
    free(this);  
    RETURN_ERROR("getting input metadata", "OpenInput", NULL); 
  }

  /* Get SDS information and start SDS access */
  for (ib = 0; ib < this->nband; ib++) {  /* image data */
    this->sds[ib].name = NULL;
    this->sds[ib].dim[0].name = NULL;
    this->sds[ib].dim[1].name = NULL;
    this->buf[ib] = NULL;
  }

  /* thermal data */
  this->therm_sds.name = NULL;
  this->therm_sds.dim[0].name = NULL;
  this->therm_sds.dim[1].name = NULL;
  this->therm_buf = NULL;

  /* Loop through the TOA image bands and obtain the SDS information */
  for (ib = 0; ib < this->nband; ib++) {
    if (sprintf(sds_name, "%s%d", SDS_PREFIX, this->meta.band[ib]) < 0) {
      error_string = "creating SDS name";
      break;
    }

    this->sds[ib].name = DupString(sds_name);
    if (this->sds[ib].name == NULL) {
      error_string = "setting SDS name";
      break;
    }

    if (!GetSDSInfo(this->sds_cal_file_id, &this->sds[ib])) {
      error_string = "getting sds info";
      break;
    }

    /* Check rank */
    if (this->sds[ib].rank != 2) {
      error_string = "invalid rank";
      break;
    }

    /* Check SDS type */
    if (this->sds[ib].type != DFNT_INT16) {
      error_string = "invalid number type";
      break;
    }

    /* Get dimensions */
    for (ir = 0; ir < this->sds[ib].rank; ir++) {
      dim[ir] = &this->sds[ib].dim[ir];
      if (!GetSDSDimInfo(this->sds[ib].id, dim[ir], ir)) {
        error_string = "getting dimensions";
        break;
      }
    }

    if (error_string != NULL) break;

    /* Save and check line and sample dimensions */
    if (ib == 0) {
      this->size.l = dim[0]->nval;
      this->size.s = dim[1]->nval;
    } else {
      if (this->size.l != dim[0]->nval) {
        error_string = "all line dimensions do not match";
        break;
      }
      if (this->size.s != dim[1]->nval) {
        error_string = "all sample dimensions do not match";
        break;
      }
    }

    /* If this is the first image band, read the fill value */
    attr.type = DFNT_FLOAT32;
    attr.nval = 1;
    attr.name = INPUT_FILL_VALUE;
    if (!GetAttrDouble(this->sds[ib].id, &attr, dval))
      RETURN_ERROR("reading band SDS attribute (fill value)", "OpenInput",
        false);
    if (attr.nval != 1) 
      RETURN_ERROR("invalid number of values (fill value)", "OpenInput", false);
    this->meta.fill = dval[0];
  }  /* for ib */

  /* Get the input metadata */
  if (!GetInputMeta2(this)) {
    free(this->lndth_name);
    free(this->lndcal_name);
    free(this);  
    RETURN_ERROR("getting input metadata", "OpenInput", NULL); 
  }

  /* For the single thermal band, obtain the SDS information */
  strcpy (sds_name, "band6");
  this->therm_sds.name = DupString(sds_name);

  if (this->therm_sds.name == NULL)
    error_string = "setting thermal SDS name";

  if (!GetSDSInfo(this->sds_sr_file_id, &this->therm_sds))
    error_string = "getting thermal sds info";



  /* Check rank */
  if (this->therm_sds.rank != 2)
    error_string = "invalid thermal rank";

  /* Check SDS type */
  if (this->therm_sds.type != DFNT_INT16)
    error_string = "invalid thermal number type";

  /* Get dimensions */
  for (ir = 0; ir < this->therm_sds.rank; ir++) {
    dim[ir] = &this->therm_sds.dim[ir];
    if (!GetSDSDimInfo(this->therm_sds.id, dim[ir], ir))
      error_string = "getting thermal dimensions";
  }

   /* Save and check toa image line and sample dimensions */
   this->toa_size.l = dim[0]->nval;
   this->toa_size.s = dim[1]->nval;

   /* Set thermal band BT scale_factor and offset */
   this->meta.therm_scale_factor_ref = 0.1;
   this->meta.therm_add_offset_ref = 0.0;

#if 0
  /* Get the input metadata */
  if (!GetInputMeta3(this)) {
    free(this->lndth_name);
    free(this->lndcal_name);
    free(this);  
    RETURN_ERROR("getting input metadata", "OpenInput", NULL); 
  }
#endif
  /* Allocate input buffers.  Thermal band only has one band.  Image and QA
     buffers have multiple bands. */
  buf = (int16 *)calloc((size_t)(this->size.s * this->nband), sizeof(int16));
  if (buf == NULL)
    error_string = "allocating input buffer";
  else {
    this->buf[0] = buf;
    for (ib = 1; ib < this->nband; ib++)
      this->buf[ib] = this->buf[ib - 1] + this->size.s;
  }

  this->therm_buf = (int16 *)calloc((size_t)(this->size.s), sizeof(int16));
  if (this->therm_buf == NULL)
    error_string = "allocating input thermal buffer";

  if (error_string != NULL) {
    FreeInput (this);
    CloseInput (this);
    RETURN_ERROR(error_string, "OpenInput", NULL);
  }

    /* Get metadata info as needed */
    status = getMeta(lndmeta_name, this);
    if (status != 0)
        error_string = "Getting metadata";

    if (error_string != NULL) {
        FreeInput (this);
        CloseInput (this);
        RETURN_ERROR(error_string, "OpenInput", NULL);
    }

  return this;
}


/******************************************************************************
!Description: 'CloseInput' ends SDS access and closes the input file.
 
!Input Parameters:
 this           'input' data structure

!Output Parameters:
 this           'input' data structure; the following fields are modified:
                   open
 (returns)      status:
                  'true' = okay
                  'false' = error return

!Team Unique Header:

!Design Notes:
******************************************************************************/
bool CloseInput(Input_t *this)
{
  int ib;

  if (!this->open)
    RETURN_ERROR("file not open", "CloseInput", false);

  /* Close image SDSs */
  for (ib = 0; ib < this->nband; ib++) {
    if (SDendaccess(this->sds[ib].id) == HDF_ERROR) 
      RETURN_ERROR("ending sds access", "CloseInput", false);
  }

  /* Close thermal SDS */
  if (SDendaccess(this->therm_sds.id) == HDF_ERROR) 
    RETURN_ERROR("ending thermal sds access", "CloseInput", false);

  /* Close the HDF file itself */
  SDend(this->sds_sr_file_id);
  SDend(this->sds_cal_file_id);
  this->open = false;

  return true;
}


/******************************************************************************
!Description: 'FreeInput' frees the 'input' data structure memory.
!Input Parameters:
 this           'input' data structure

!Output Parameters:
 (returns)      status:
                  'true' = okay (always returned)

!Team Unique Header:

!Design Notes:
******************************************************************************/
bool FreeInput(Input_t *this)
{
  int ib, ir;

  if (this->open) 
    RETURN_ERROR("file still open", "FreeInput", false);

  if (this != NULL) {
    /* Free image band SDSs */
    for (ib = 0; ib < this->nband; ib++) {
      for (ir = 0; ir < this->sds[ib].rank; ir++) {
        if (this->sds[ib].dim[ir].name != NULL) 
          free(this->sds[ib].dim[ir].name);
      }
      if (this->sds[ib].name != NULL) 
        free(this->sds[ib].name);
    }

    /* Free thermal band SDS */
    for (ir = 0; ir < this->therm_sds.rank; ir++) {
      if (this->therm_sds.dim[ir].name != NULL) 
        free(this->therm_sds.dim[ir].name);
    }
    if (this->therm_sds.name != NULL) 
      free(this->therm_sds.name);

    /* Free the data buffers */
    if (this->buf[0] != NULL)
      free(this->buf[0]);
    if (this->therm_buf != NULL)
      free(this->therm_buf);

    if (this->lndth_name != NULL)
      free(this->lndth_name);
    if (this->lndcal_name != NULL)
      free(this->lndcal_name);
    free(this);
  } /* end if */

  return true;
}


/******************************************************************************
!Description: 'GetInputLine' reads the TOA reflectance data for the current
   band and line
 
!Input Parameters:
 this           'input' data structure
 iband          current band to be read (0-based)
 iline          current line to be read (0-based)

!Output Parameters:
 this           'input' data structure; the following fields are modified:
                   buf -- contains the line read
 (returns)      status:
                  'true' = okay
                  'false' = error return

!Team Unique Header:

!Design Notes:
******************************************************************************/
bool GetInputLine(Input_t *this, int iband, int iline)
{
  int32 start[MYHDF_MAX_RANK], nval[MYHDF_MAX_RANK];
  void *buf = NULL;

  /* Check the parameters */
  if (this == (Input_t *)NULL) 
    RETURN_ERROR("invalid input structure", "GetIntputLine", false);
  if (!this->open)
    RETURN_ERROR("file not open", "GetInputLine", false);
  if (iband < 0 || iband >= this->nband)
    RETURN_ERROR("invalid band number", "GetInputLine", false);
  if (iline < 0 || iline >= this->size.l)
    RETURN_ERROR("invalid line number", "GetInputLine", false);

  /* Read the data */
  start[0] = iline;
  start[1] = 0;
  nval[0] = 1;
  nval[1] = this->size.s;
  buf = (void *)this->buf[iband];

  if (SDreaddata(this->sds[iband].id, start, NULL, nval, buf) == HDF_ERROR)
    RETURN_ERROR("reading input", "GetInputLine", false);

  return true;
}

/******************************************************************************
!Description: 'GetInputThermLine' reads the thermal brightness data for the
current line
 
!Input Parameters:
 this           'input' data structure
 iline          current line to be read (0-based)

!Output Parameters:
 this           'input' data structure; the following fields are modified:
                   therm_buf -- contains the line read
 (returns)      status:
                  'true' = okay
                  'false' = error return

!Team Unique Header:

!Design Notes:
******************************************************************************/
bool GetInputThermLine(Input_t *this, int iline)
{
  int32 start[MYHDF_MAX_RANK], nval[MYHDF_MAX_RANK];
  void *buf = NULL;

  /* Check the parameters */
  if (this == (Input_t *)NULL) 
    RETURN_ERROR("invalid input structure", "GetIntputThermLine", false);
  if (!this->open)
    RETURN_ERROR("file not open", "GetInputThermLine", false);
  if (iline < 0 || iline >= this->size.l)
    RETURN_ERROR("invalid line number", "GetInputThermLine", false);

  /* Read the data */
  start[0] = iline;
  start[1] = 0;
  nval[0] = 1;
  nval[1] = this->size.s;
  buf = (void *)this->therm_buf;

  if (SDreaddata(this->therm_sds.id, start, NULL, nval, buf) == HDF_ERROR)
    RETURN_ERROR("reading input", "GetInputThermLine", false);

  return true;
}


/******************************************************************************
!Description: 'GetInputMeta' reads the metadata for input HDF file
 
!Input Parameters:
 this           'input' data structure

!Output Parameters:
 this           'input' data structure; metadata fields are populated
 (returns)      status:
                  'true' = okay
                  'false' = error return

!Team Unique Header:

!Design Notes:
******************************************************************************/
bool GetInputMeta(Input_t *this) 
{
  Myhdf_attr_t attr;
  double dval[NBAND_REFL_MAX];
  char date[MAX_DATE_LEN + 1];
  int ib;
  Input_meta_t *meta = NULL;
  char *error_string = NULL;

  /* Check the parameters */
  if (!this->open)
    RETURN_ERROR("file not open", "GetInputMeta", false);

  meta = &this->meta;

  /* Read the metadata */
  attr.type = DFNT_CHAR8;
  attr.nval = MAX_STR_LEN;
  attr.name = INPUT_PROVIDER;
  if (!GetAttrString(this->sds_sr_file_id, &attr, this->meta.provider))
    RETURN_ERROR("reading attribute (data provider)", "GetInputMeta", false);

  attr.type = DFNT_CHAR8;
  attr.nval = MAX_STR_LEN;
  attr.name = INPUT_SAT;
  if (!GetAttrString(this->sds_sr_file_id, &attr, this->meta.sat))
    RETURN_ERROR("reading attribute (instrument)", "GetInputMeta", false);

  attr.type = DFNT_CHAR8;
  attr.nval = MAX_STR_LEN;
  attr.name = INPUT_INST;
  if (!GetAttrString(this->sds_sr_file_id, &attr, this->meta.inst))
    RETURN_ERROR("reading attribute (instrument)", "GetInputMeta", false);

  attr.type = DFNT_CHAR8;
  attr.nval = MAX_DATE_LEN;
  attr.name = INPUT_ACQ_DATE;
  if (!GetAttrString(this->sds_sr_file_id, &attr, date))
    RETURN_ERROR("reading attribute (acquisition date)", "GetInputMeta", false);
  if (!DateInit(&meta->acq_date, date, DATE_FORMAT_DATEA_TIME))
    RETURN_ERROR("converting acquisition date", "GetInputMeta", false);

  attr.type = DFNT_CHAR8;
  attr.nval = MAX_DATE_LEN;
  attr.name = INPUT_PROD_DATE;
  if (!GetAttrString(this->sds_cal_file_id, &attr, date))
    RETURN_ERROR("reading attribute (production date)", "GetInputMeta", false);
  if (!DateInit(&meta->prod_date, date, DATE_FORMAT_DATEA_TIME))
    RETURN_ERROR("converting production date", "GetInputMeta", false);

  attr.type = DFNT_FLOAT32;
  attr.nval = 1;
  attr.name = INPUT_SUN_ZEN;
  if (!GetAttrDouble(this->sds_sr_file_id, &attr, dval))
    RETURN_ERROR("reading attribute (solar zenith)", "GetInputMeta", false);
  if (attr.nval != 1) 
    RETURN_ERROR("invalid number of values (solar zenith)", 
                  "GetInputMeta", false);
  if (dval[0] < -90.0  ||  dval[0] > 90.0)
    RETURN_ERROR("solar zenith angle out of range", "GetInputMeta", false);
  meta->sun_zen = (float)(dval[0]);

  attr.type = DFNT_FLOAT32;
  attr.nval = 1;
  attr.name = INPUT_SUN_AZ;
  if (!GetAttrDouble(this->sds_sr_file_id, &attr, dval))
    RETURN_ERROR("reading attribute (solar azimuth)", "GetInputMeta", false);
  if (attr.nval != 1) 
    RETURN_ERROR("invalid number of values (solar azimuth)", 
                 "GetInputMeta", false);
  if (dval[0] < -360.0  ||  dval[0] > 360.0)
    RETURN_ERROR("solar azimuth angle out of range", "GetInputMeta", false);
  meta->sun_az = (float)(dval[0]);

  attr.type = DFNT_CHAR8;
  attr.nval = MAX_STR_LEN;
  attr.name = INPUT_WRS_SYS;
  if (!GetAttrString(this->sds_sr_file_id, &attr, this->meta.wrs_sys))
    RETURN_ERROR("reading attribute (WRS system)", "GetInputMeta", false);

  attr.type = DFNT_INT16;
  attr.nval = 1;
  attr.name = INPUT_WRS_PATH;
  if (!GetAttrDouble(this->sds_sr_file_id, &attr, dval))
    RETURN_ERROR("reading attribute (WRS path)", "GetInputMeta", false);
  if (attr.nval != 1) 
    RETURN_ERROR("invalid number of values (WRS path)", "GetInputMeta", false);
  meta->path = (int)floor(dval[0] + 0.5);
  if (meta->path < 1) 
    RETURN_ERROR("WRS path out of range", "GetInputMeta", false);

  attr.type = DFNT_INT16;
  attr.nval = 1;
  attr.name = INPUT_WRS_ROW;
  if (!GetAttrDouble(this->sds_sr_file_id, &attr, dval))
    RETURN_ERROR("reading attribute (WRS row)", "GetInputMeta", false);
  if (attr.nval != 1) 
    RETURN_ERROR("invalid number of values (WRS row)", "GetInputMeta", false);
  meta->row = (int)floor(dval[0] + 0.5);
  if (meta->row < 1) 
    RETURN_ERROR("WRS path out of range", "GetInputMeta", false);
#if 0
    /* Get the bounding coordinates if they are available */
    meta->bounds.is_fill = false;
    attr.type = DFNT_FLOAT32;
    attr.nval = 1;
    attr.name = INPUT_WEST_BOUND;
    if (!GetAttrDouble(this->sds_sr_file_id, &attr, dval))
    {
        RETURN_ERROR("reading west bound", "GetInputMeta", false);
        meta->bounds.is_fill = true;
    }
    if (attr.nval != 1) 
    {
        RETURN_ERROR("invalid number of west bound", "GetInputMeta", false);
        meta->bounds.is_fill = true;
    }
    meta->bounds.min_lon = dval[0];

    attr.type = DFNT_FLOAT32;
    attr.nval = 1;
    attr.name = INPUT_EAST_BOUND;
    if (!GetAttrDouble(this->sds_sr_file_id, &attr, dval))
    {
        RETURN_ERROR("reading east bound", "GetInputMeta", false);
        meta->bounds.is_fill = true;
    }
    if (attr.nval != 1) 
    {
        RETURN_ERROR("invalid number of east bound", "GetInputMeta", false);
        meta->bounds.is_fill = true;
    }
    meta->bounds.max_lon = dval[0];

    attr.type = DFNT_FLOAT32;
    attr.nval = 1;
    attr.name = INPUT_NORTH_BOUND;
    if (!GetAttrDouble(this->sds_sr_file_id, &attr, dval))
    {
        RETURN_ERROR("reading north bound", "GetInputMeta", false);
        meta->bounds.is_fill = true;
    }
    if (attr.nval != 1) 
    {
        RETURN_ERROR("invalid number of north bound", "GetInputMeta", false);
        meta->bounds.is_fill = true;
    }
    meta->bounds.max_lat = dval[0];

    attr.type = DFNT_FLOAT32;
    attr.nval = 1;
    attr.name = INPUT_SOUTH_BOUND;
    if (!GetAttrDouble(this->sds_sr_file_id, &attr, dval))
    {
        RETURN_ERROR("reading south bound", "GetInputMeta", false);
        meta->bounds.is_fill = true;
    }
    if (attr.nval != 1) 
    {
        RETURN_ERROR("invalid number of south bound", "GetInputMeta", false);
        meta->bounds.is_fill = true;
    }
    meta->bounds.min_lat = dval[0];
#endif

  attr.type = DFNT_INT8;
  attr.nval = 1;
  attr.name = INPUT_NBAND;
  if (!GetAttrDouble(this->sds_cal_file_id, &attr, dval))
    RETURN_ERROR("reading attribute (number of bands)", "GetInputMeta", false);
  if (attr.nval != 1) 
    RETURN_ERROR("invalid number of values (number of bands)", 
                 "GetInputMeta", false);
  this->nband = (int)floor(dval[0] + 0.5);
  if (this->nband < 1  ||  this->nband > NBAND_REFL_MAX) 
    RETURN_ERROR("number of bands out of range", "GetInputMeta", false);

  attr.type = DFNT_INT8;
  attr.nval = this->nband;
  attr.name = INPUT_BANDS;
  if (!GetAttrDouble(this->sds_cal_file_id, &attr, dval))
    RETURN_ERROR("reading attribute (band numbers)", "GetInputMeta", false);
  if (attr.nval != this->nband) 
    RETURN_ERROR("invalid number of values (band numbers)", 
                 "GetInputMeta", false);
  for (ib = 0; ib < this->nband; ib++) {
    meta->band[ib] = (int)floor(dval[ib] + 0.5);
    if (meta->band[ib] < 1)
      RETURN_ERROR("band number out of range", "GetInputMeta", false);
  }

  /* Check WRS path/rows */
  error_string = (char *)NULL;

  if (!strcmp (meta->wrs_sys, "1")) {
    if (meta->path > N_LSAT_WRS1_PATHS)
      error_string = "WRS path number out of range";
    else if (meta->row > N_LSAT_WRS1_ROWS)
      error_string = "WRS row number out of range";
  } else if (!strcmp (meta->wrs_sys, "2")) {
    if (meta->path > N_LSAT_WRS2_PATHS)
      error_string = "WRS path number out of range";
    else if (meta->row > N_LSAT_WRS2_ROWS)
      error_string = "WRS row number out of range";
  } else
    error_string = "invalid WRS system";

  if (error_string != (char *)NULL)
    RETURN_ERROR(error_string, "GetInputMeta", false);

  return true;
}

bool GetInputMeta2(Input_t *this) 
{
  Myhdf_attr_t attr;
  double dval[NBAND_REFL_MAX];
  int ib;

  attr.type = DFNT_CHAR8;
  attr.nval = MAX_STR_LEN;
  attr.name = INPUT_UNITS;
  if (!GetAttrString(this->sds[0].id, &attr, this->meta.unit_ref))
    RETURN_ERROR("reading attribute (unit ref)", "GetInputMeta", false);

  attr.type = DFNT_INT16;
  attr.nval = 2;
  attr.name = INPUT_VALID_RANGE;
  if (!GetAttrDouble(this->sds[0].id, &attr, dval))
    RETURN_ERROR("reading attribute (valid range ref)", "GetInputMeta", false);
  if (attr.nval != 2) 
    RETURN_ERROR("invalid number of values (valid range ref)", "GetInputMeta", false);
  this->meta.valid_range_ref[0] = (float)dval[0];
  this->meta.valid_range_ref[1] = (float)dval[1];

  for (ib = 0; ib < this->nband; ib++) {
      attr.type = DFNT_INT16;
      attr.nval = 1;
      attr.name = INPUT_SATU_VALUE;
      if (!GetAttrDouble(this->sds[ib].id, &attr, dval))
         RETURN_ERROR("reading attribute (saturate value ref)", "GetInputMeta",
                     false);
      if (attr.nval != 1) 
         RETURN_ERROR("invalid number of values (saturate value ref)", 
              "GetInputMeta", false);
       this->meta.satu_value_ref[ib] = (int)dval[0];
  }

  attr.type = DFNT_FLOAT64;
  attr.nval = 1;
  attr.name = INPUT_SCALE_FACTOR;
  if (!GetAttrDouble(this->sds[0].id, &attr, dval))
    RETURN_ERROR("reading attribute (scale factor ref)", "GetInputMeta", false);
  if (attr.nval != 1) 
    RETURN_ERROR("invalid number of values (scale factor ref)", "GetInputMeta", false);
  this->meta.scale_factor_ref = (float)dval[0];

  attr.type = DFNT_FLOAT64;
  attr.nval = 1;
  attr.name = INPUT_ADD_OFFSET;
  if (!GetAttrDouble(this->sds[0].id, &attr, dval))
    RETURN_ERROR("reading attribute (add offset ref)", "GetInputMeta", false);
  if (attr.nval != 1) 
    RETURN_ERROR("invalid number of values (add offset ref)", "GetInputMeta", false);
  this->meta.add_offset_ref = (float)dval[0];

  attr.type = DFNT_FLOAT64;
  attr.nval = 1;
  attr.name = INPUT_SCALE_FACTOR_ERR;
  if (!GetAttrDouble(this->sds[0].id, &attr, dval))
    RETURN_ERROR("reading attribute (scale factor err  ref)", "GetInputMeta", false);
  if (attr.nval != 1) 
    RETURN_ERROR("invalid number of values (scale factor err ref)", "GetInputMeta", false);
  this->meta.scale_factor_err_ref = (float)dval[0];

  attr.type = DFNT_FLOAT64;
  attr.nval = 1;
  attr.name = INPUT_ADD_OFFSET_ERR;
  if (!GetAttrDouble(this->sds[0].id, &attr, dval))
    RETURN_ERROR("reading attribute (add offset err ref)", "GetInputMeta", false);
  if (attr.nval != 1) 
    RETURN_ERROR("invalid number of values (add offset err ref)", "GetInputMeta", false);
  this->meta.add_offset_err_ref = (float)dval[0];

  attr.type = DFNT_FLOAT32;
  attr.nval = 1;
  attr.name = INPUT_CALIBRATED_NT;
  if (!GetAttrDouble(this->sds[0].id, &attr, dval))
    RETURN_ERROR("reading attribute (calibrated NT ref)", "GetInputMeta", false);
  if (attr.nval != 1) 
    RETURN_ERROR("invalid number of values (calibrated NT ref)", "GetInputMeta", false);
  this->meta.calibrated_nt_ref = (float)dval[0];

  return true;
}

bool GetInputMeta3(Input_t *this) 
{
  Myhdf_attr_t attr;
  double dval[NBAND_REFL_MAX];

  attr.type = DFNT_CHAR8;
  attr.nval = MAX_STR_LEN;
  attr.name = INPUT_UNITS;
  if (!GetAttrString(this->sds_sr_file_id, &attr, this->meta.therm_unit_ref))
    RETURN_ERROR("reading attribute (unit ref)", "GetInputMeta", false);

  attr.type = DFNT_INT16;
  attr.nval = 2;
  attr.name = INPUT_VALID_RANGE;
  if (!GetAttrDouble(this->sds_sr_file_id, &attr, dval))
    RETURN_ERROR("reading attribute (valid range ref)", "GetInputMeta", false);
  if (attr.nval != 2) 
    RETURN_ERROR("invalid number of values (valid range ref)", "GetInputMeta", false);
  this->meta.therm_valid_range_ref[0] = (float)dval[0];
  this->meta.therm_valid_range_ref[1] = (float)dval[1];

  attr.type = DFNT_INT16;
  attr.nval = 1;
  attr.name = INPUT_SATU_VALUE;
  if (!GetAttrDouble(this->sds_sr_file_id, &attr, dval))
    RETURN_ERROR("reading attribute (saturate value ref)", "GetInputMeta", false);
  if (attr.nval != 1) 
    RETURN_ERROR("invalid number of values (saturate value ref)", "GetInputMeta", false);
  this->meta.therm_satu_value_ref = (float)dval[0];

  attr.type = DFNT_FLOAT64;
  attr.nval = 1;
  attr.name = INPUT_SCALE_FACTOR;
  if (!GetAttrDouble(this->sds_sr_file_id, &attr, dval))
    RETURN_ERROR("reading attribute (scale factor ref)", "GetInputMeta", false);
  if (attr.nval != 1) 
    RETURN_ERROR("invalid number of values (scale factor ref)", "GetInputMeta", false);
  this->meta.therm_scale_factor_ref = (float)dval[0];

  attr.type = DFNT_FLOAT64;
  attr.nval = 1;
  attr.name = INPUT_ADD_OFFSET;
  if (!GetAttrDouble(this->sds_sr_file_id, &attr, dval))
    RETURN_ERROR("reading attribute (add offset ref)", "GetInputMeta", false);
  if (attr.nval != 1) 
    RETURN_ERROR("invalid number of values (add offset ref)", "GetInputMeta", false);
  this->meta.therm_add_offset_ref = (float)dval[0];

  attr.type = DFNT_FLOAT64;
  attr.nval = 1;
  attr.name = INPUT_SCALE_FACTOR_ERR;
  if (!GetAttrDouble(this->sds_sr_file_id, &attr, dval))
    RETURN_ERROR("reading attribute (scale factor err  ref)", "GetInputMeta", false);
  if (attr.nval != 1) 
    RETURN_ERROR("invalid number of values (scale factor err ref)", "GetInputMeta", false);
  this->meta.therm_scale_factor_err_ref = (float)dval[0];

  attr.type = DFNT_FLOAT64;
  attr.nval = 1;
  attr.name = INPUT_ADD_OFFSET_ERR;
  if (!GetAttrDouble(this->sds_sr_file_id, &attr, dval))
    RETURN_ERROR("reading attribute (add offset err ref)", "GetInputMeta", false);
  if (attr.nval != 1) 
    RETURN_ERROR("invalid number of values (add offset err ref)", "GetInputMeta", false);
  this->meta.therm_add_offset_err_ref = (float)dval[0];

  attr.type = DFNT_FLOAT32;
  attr.nval = 1;
  attr.name = INPUT_CALIBRATED_NT;
  if (!GetAttrDouble(this->sds_sr_file_id, &attr, dval))
    RETURN_ERROR("reading attribute (calibrated NT ref)", "GetInputMeta", false);
  if (attr.nval != 1) 
    RETURN_ERROR("invalid number of values (calibrated NT ref)", "GetInputMeta", false);
  this->meta.therm_calibrated_nt_ref = (float)dval[0];

  return true;
}

#define DATE_STRING_LEN (10)
#define TIME_STRING_LEN (15)
#define NLINE_MAX (20000)
#define NSAMP_MAX (20000)

#if 0
bool GetHeaderInput(Input_t *this, char *file_header_name, Param_t *param) {
  char *error_string = (char *)NULL;
  int ib;
  FILE *fp;
  Key_t key;
  int nband_iband, nband_gain_set, nband_gain, nband_bias, nband_file_name;
  char acq_date[DATE_STRING_LEN + 1];
  char prod_date[DATE_STRING_LEN + 1];
  char acq_time[TIME_STRING_LEN + 1];
  char temp[MAX_STR_LEN + 1];
  bool got_start, got_end;
  int len;
  char line[MAX_STR_LEN + 1];
  Header_key_t header_key;
  int i;
  bool flag_gain_set, flag_gain, flag_bias;

  /* Populate the data structure */

  this->file_header_name = DupString(file_header_name);
  if (this->file_header_name == (char *)NULL) {
    free(this);
    RETURN_ERROR("duplicating file name", "GetHeaderInput", false);
  }

  this->file_type = INPUT_TYPE_NULL;
  this->meta.provider = PROVIDER_NULL;
  this->meta.sat = SAT_NULL;
  this->meta.inst = INST_NULL;
  this->meta.acq_date.fill = true;
  this->meta.time_fill = true;
  this->meta.prod_date.fill = true;
  this->meta.sun_zen = ANGLE_FILL;
  this->meta.sun_az = ANGLE_FILL;
  this->meta.wrs_sys = (Wrs_t)WRS_FILL;
  this->meta.ipath = -1;
  this->meta.irow = -1;
  this->meta.fill = INPUT_FILL;
  this->nband = 0;
  this->nband_th = 0;
  this->size.s = this->size.l = -1;
  for (ib = 0; ib < NBAND_REFL_MAX; ib++) {
    this->meta.iband[ib] = -1;
    this->meta.gain_set[ib] = GAIN_NULL;
    this->meta.gain[ib] = GAIN_BIAS_FILL;
    this->meta.bias[ib] = GAIN_BIAS_FILL;
    this->file_name[ib] = (char *)NULL;
    this->open[ib] = false;
    this->fp_bin[ib] = (FILE *)NULL;
    this->fp_tiff[ib] = (void *)NULL;
    this->buf[ib] = (void *)NULL;
  }
  this->nband_th = 0;
  this->open_th = false;
  this->meta.gain_th = GAIN_BIAS_FILL;
  this->meta.bias_th = GAIN_BIAS_FILL;
  this->dnout= false;
  this->dn_map[0]= 0.0;
  this->dn_map[1]= 0.0;
  this->dn_map[2]= 0.0;
  this->dn_map[3]= 0.0;
  this->file_name_th  = (char *)NULL;
  this->fp_bin_th  = (FILE *)NULL;
  this->fp_tiff_th  = (void *)NULL;
  this->buf_th  = (void *)NULL;

  /* Open the header file */
  
  if ((fp = fopen(this->file_header_name, "r")) == (FILE *)NULL) {
    free(this->file_header_name);
    free(this);
    RETURN_ERROR("unable to open header file", "GetHeaderInput", false);
  }

  /* Parse the header file */

  got_start = got_end = false;
  nband_iband = nband_gain_set = -1;
  nband_gain = nband_bias = nband_file_name = -1;
  acq_date[0] = acq_time[0] = '\0';
  prod_date[0] = '\0';
  flag_gain_set = flag_gain = flag_bias = false;

  while((len = GetLine(fp, line)) > 0) {
    if (!StringParse(line, &key)) {
      sprintf(temp, "parsing header file; line = %s", line);
      error_string = temp;
      break;
    }
    if (key.len_key <= 0) continue;
    if (key.key[0] == '#') continue;
    header_key = (Header_key_t)
       KeyString(key.key, key.len_key, Header_string, 
		 (int)HEADER_NULL, (int)HEADER_MAX);

    if (header_key == HEADER_NULL) {
      key.key[key.len_key] = '\0';
    }
    if (!got_start) {
      if (header_key == HEADER_START) {
        if (key.nval != 0) {
	  error_string = "no value expected";
	  break;
	}
        got_start = true;
	continue;
      } else {
        error_string  = "no start key in header";
        break;
      }
    }

    /* Get the value for each keyword */

    switch (header_key) {

      case HEADER_FILE_TYPE:
        if (key.nval <= 0) {
	  error_string = "no file type value";
	  break; 
	} else if (key.nval > 1) {
	  error_string = "too many file type values";
	  break; 
	}
        this->file_type = (Input_type_t)
	  KeyString(key.value[0], key.len_value[0],
	            Input_type_string, 
		    (int)INPUT_TYPE_NULL, (int)INPUT_TYPE_MAX);
        if (this->file_type == INPUT_TYPE_NULL) {
	  key.value[0][key.len_value[0]] = '\0';
          sprintf(temp, "invalid file type; value = %s", key.value[0]);
          error_string = temp;
	}
        break;

      case HEADER_PROIVDER:
        if (key.nval <= 0) {
	  error_string = "no provider value";
	  break; 
	} else if (key.nval > 1) {
	  error_string = "too many provider values";
	  break; 
	}
        this->meta.provider = (Provider_t)
	  KeyString(key.value[0], key.len_value[0],
	            Provider_string, (int)PROVIDER_NULL, (int)PROVIDER_MAX);
        if (this->meta.provider == PROVIDER_NULL) {
	  key.value[0][key.len_value[0]] = '\0';
          sprintf(temp, "invalid data provider; value = %s", key.value[0]);
          error_string = temp;
	}
        break;

      case HEADER_SAT:
        if (key.nval <= 0) {
	  error_string = "no satellite value";
	  break; 
	} else if (key.nval > 1) {
	  error_string = "too many satellite values";
	  break; 
	}
        this->meta.sat = (Sat_t)
	  KeyString(key.value[0], key.len_value[0],
	            Sat_string, (int)SAT_NULL, (int)SAT_MAX);
        if (this->meta.sat == SAT_NULL) {
	  key.value[0][key.len_value[0]] = '\0';
          sprintf(temp, "invalid satellite; value = %s", key.value[0]);
          error_string = temp;
	}
        break;

      case HEADER_INST:
        if (key.nval <= 0) {
	  error_string = "no instrument value";
	  break; 
	} else if (key.nval > 1) {
	  error_string = "too many instrument values";
	  break; 
	}
        this->meta.inst = (Inst_t)
	  KeyString(key.value[0], key.len_value[0],
	            Inst_string, (int)INST_NULL, (int)INST_MAX);
        if (this->meta.inst == INST_NULL) {
	  key.value[0][key.len_value[0]] = '\0';
          sprintf(temp, "invalid satellite; value = %s", key.value[0]);
          error_string = temp;
	}
        break;

      case HEADER_ACQ_DATE:
        if (key.nval <= 0) {
	  error_string = "no acquisition date value";
	  break; 
	} else if (key.nval > 1) {
	  error_string = "too many acquisition date values";
	  break; 
	}
        if (key.len_value[0] != DATE_STRING_LEN) {
	  error_string = "invalid acquisition date string";
	  break;
	}
	strncpy(acq_date, key.value[0], key.len_value[0]);
	acq_date[key.len_value[0]] = '\0';
        break;

      case HEADER_ACQ_TIME:
        if (key.nval <= 0) {
	  error_string = "no acquisition time value";
	  break; 
	} else if (key.nval > 1) {
	  error_string = "too many acquisition time values";
	  break; 
	}
        if (key.len_value[0] < 8  ||
	    key.len_value[0] > TIME_STRING_LEN) {
	  error_string = "invalid time string";
	  break;
	}
	strncpy(acq_time, key.value[0], key.len_value[0]);
	acq_time[key.len_value[0]] = '\0';
        break;

      case HEADER_PROD_DATE:
        if (key.nval <= 0) {
	  error_string = "no production date value";
	  break; 
	} else if (key.nval > 1) {
	  error_string = "too many production date values";
	  break; 
	}
        if (key.len_value[0] >0 && 
            key.len_value[0] != DATE_STRING_LEN) {
	  error_string = "invalid production date string";
	  break; 
	}
        if (key.len_value[0] >0 ){
  	  strncpy(prod_date, key.value[0], key.len_value[0]);
	  prod_date[key.len_value[0]] = '\0';
	}
        break;

      case HEADER_SUN_ZEN:
        if (key.nval <= 0) {
	  error_string = "no solar zenith angle value";
	  break; 
	} else if (key.nval > 1) {
	  error_string = "too many solar zenith angle values";
	  break; 
	}
        if (key.len_value[0] < 1) {
	  error_string = "invalid solar zenith angle string";
	  break;
	}
	strncpy(temp, key.value[0], key.len_value[0]);
        temp[key.len_value[0]] = '\0';

	if (sscanf(temp, "%f", &this->meta.sun_zen) != 1) {
	  error_string = "converting solar zenith angle";
	  break;
	}
	if (this->meta.sun_zen < -90.0  ||  
	    this->meta.sun_zen >  90.0) {
	  error_string = "solar zenith angle out of range";
	  break;
	}
	this->meta.sun_zen *= RAD;
        break;

      case HEADER_SUN_AZ:
        if (key.nval <= 0) {
	  error_string = "no solar azimuth angle value";
	  break; 
	} else if (key.nval > 1) {
	  error_string = "too many solar azimuth angle values";
	  break; 
	}
        if (key.len_value[0] < 1) {
	  error_string = "invalid solar azimuth angle string";
	  break;
	}
	strncpy(temp, key.value[0], key.len_value[0]);
        temp[key.len_value[0]] = '\0';

	if (sscanf(temp, "%f", &this->meta.sun_az) != 1) {
	  error_string = "converting solar azimuth angle";
	  break;
	}
	if (this->meta.sun_az < -360.0  ||  
	    this->meta.sun_az >  360.0) {
	  error_string = "solar azimuth angle out of range";
	  break;
	}
	this->meta.sun_az *= RAD;
        break;

      case HEADER_WRS_SYS:
        if (key.nval <= 0) {
	  error_string = "no WRS system value";
	  break; 
	} else if (key.nval > 1) {
	  error_string = "too many WRS system values";
	  break; 
	}
        this->meta.wrs_sys = (Wrs_t)
	  KeyString(key.value[0], key.len_value[0],
	            Wrs_string, (int)WRS_NULL, (int)WRS_MAX);
        if (this->meta.wrs_sys == WRS_NULL) {
	  key.value[0][key.len_value[0]] = '\0';
          sprintf(temp, "invalid WRS system; value = %s", key.value[0]);
          error_string = temp;
	}
        break;

      case HEADER_WRS_PATH:
        if (key.nval <= 0) {
	  error_string = "no WRS path value";
	  break; 
	} else if (key.nval > 1) {
	  error_string = "too many WRS path values";
	  break; 
	}
        if (key.len_value[0] < 1) {
	  error_string = "invalid WRS path string";
	  break;
	}
	strncpy(temp, key.value[0], key.len_value[0]);
        temp[key.len_value[0]] = '\0';

	if (sscanf(temp, "%d", &this->meta.ipath) != 1) {
	  error_string = "converting WRS path";
	  break;
	}
	if (this->meta.ipath < 1)
	  error_string = "WRS path number out of range";
        break;

      case HEADER_WRS_ROW:
        if (key.nval <= 0) {
	  error_string = "no WRS row value";
	  break; 
	} else if (key.nval > 1) {
	  error_string = "too many WRS row values";
	  break; 
	}
        if (key.len_value[0] < 1) {
	  error_string = "invalid WRS row string";
	  break;
	}
	strncpy(temp, key.value[0], key.len_value[0]);
        temp[key.len_value[0]] = '\0';

	if (sscanf(temp, "%d", &this->meta.irow) != 1) {
	  error_string = "converting WRS row";
	  break;
	}
	if (this->meta.irow < 1)
	  error_string = "WRS row number out of range";
        break;

      case HEADER_NBAND:
        if (key.nval <= 0) {
	  error_string = "no number of bands value";
	  break; 
	} else if (key.nval > 1) {
	  error_string = "too many number of bands values";
	  break; 
	}
        if (key.len_value[0] < 1) {
	  error_string = "invalid number of bands string";
	  break;
	}
	strncpy(temp, key.value[0], key.len_value[0]);
        temp[key.len_value[0]] = '\0';

	if (sscanf(temp, "%d", &this->nband) != 1) {
	  error_string = "converting number of bands";
	  break;
	}
	if (this->nband < 1  ||
	    this->nband > NBAND_REFL_MAX)
	  error_string = "number of bands out of range";
        break;

      case HEADER_BANDS:
        if (key.nval <= 0) {
	  error_string = "no band number value";
	  break; 
	} else if (key.nval > NBAND_REFL_MAX) {
	  error_string = "too many band number values";
	  break; 
	}
	for (i = 0; i < key.nval; i++) {
          if (key.len_value[i] < 1) {
	    error_string = "invalid band number string";
	    break;
	  }
	  strncpy(temp, key.value[i], key.len_value[i]);
	  temp[key.len_value[i]] = '\0';

	  if (sscanf(temp, "%d", &this->meta.iband[i]) != 1) {
	    error_string = "converting band number";
	    break;
	  }
	  if (this->meta.iband[i] < 1) {
	    error_string = "band number out of range";
	    break;
	  }
	}
	if (error_string != (char *)NULL) break;
	nband_iband = key.nval;
        break;

      case HEADER_GAIN_SET:
        if (key.nval <= 0) {
	  error_string = "no gain setting value";
	  break; 
	} else if (key.nval > NBAND_REFL_MAX) {
	  error_string = "too many gain setting values";
	  break; 
	}
	for (i = 0; i < key.nval; i++) {
          this->meta.gain_set[i] = (Gain_t)
	    KeyString(key.value[i], key.len_value[i],
	              Gain_string, (int)GAIN_NULL, (int)GAIN_MAX);
          if (this->meta.inst == (int)GAIN_NULL) {
            error_string = "invalid gain setting string";
	    break;
	  }
	}
	if (error_string != (char *)NULL) break;
	nband_gain_set = key.nval;
	flag_gain_set = true;
        break;

      case HEADER_GAIN:
        if (key.nval <= 0) {
	  error_string = "no gain values";
	  break; 
	} else if (key.nval > NBAND_REFL_MAX) {
	  error_string = "too many gain values";
	  break; 
	}
	for (i = 0; i < key.nval; i++) {
          if (key.len_value[i] < 1) {
	    error_string = "invalid gain string";
	    break;
	  }
	  strncpy(temp, key.value[i], key.len_value[i]);
          temp[key.len_value[i]] = '\0';

	  if (sscanf(temp, "%g", &this->meta.gain[i]) != 1) {
	    error_string = "converting gain";
	    break;
	  }
        }
	if (error_string != (char *)NULL) break;
	nband_gain = key.nval;
	flag_gain = true;
        break;

      case HEADER_BIAS:
        if (key.nval <= 0) {
	  error_string = "no bais values";
	  break; 
	} else if (key.nval > NBAND_REFL_MAX) {
	  error_string = "too many gain values";
	  break; 
	}
	for (i = 0; i < key.nval; i++) {
          if (key.len_value[i] < 1) {
	    error_string = "invalid bias string";
	    break;
	  }
	  strncpy(temp, key.value[i], key.len_value[i]);
          temp[key.len_value[i]] = '\0';

	  if (sscanf(temp, "%g", &this->meta.bias[i]) != 1) {
	    error_string = "converting bias";
	    break;
	  }
        }
	if (error_string != (char *)NULL) break;
	nband_bias = key.nval;
	flag_bias = true;
        break;

      case HEADER_NSAMPLE:
        if (key.nval <= 0) {
	  error_string = "no number of sample value";
	  break; 
	} else if (key.nval > 1) {
	  error_string = "too many number of sample values";
	  break; 
	}
        if (key.len_value[0] < 1) {
	  error_string = "invalid number of samples string";
	  break;
	}
	strncpy(temp, key.value[0], key.len_value[0]);
        temp[key.len_value[0]] = '\0';

	if (sscanf(temp, "%d", &this->size.s) != 1) {
	  error_string = "converting number of samples";
	  break;
	}
	if (this->size.s < 1  ||
	    this->size.s > NSAMP_MAX)
	  error_string = "sample number out of range";
        break;

      case HEADER_NLINE:
        if (key.nval <= 0) {
	  error_string = "no number of line value";
	  break; 
	} else if (key.nval > 1) {
	  error_string = "too many number of line values";
	  break; 
	}
        if (key.len_value[0] < 1) {
	  error_string = "invalid number of lines string";
	  break;
	}
	strncpy(temp, key.value[0], key.len_value[0]);
        temp[key.len_value[0]] = '\0';

	if (sscanf(temp, "%d", &this->size.l) != 1) {
	  error_string = "converting number of lines";
	  break;
	}
	if (this->size.l < 1  ||
	    this->size.l > NLINE_MAX)
	  error_string = "line number out of range";
        break;

      case HEADER_FILES:
        if (key.nval <= 0) {
	  error_string = "no file name value";
	  break; 
	} else if (key.nval > NBAND_REFL_MAX) {
	  error_string = "too many file name values";
	  break; 
	}
	for (i = 0; i < key.nval; i++) {
          if (key.len_value[i] < 1) {
	    error_string = "invalid file name string";
	    break;
	  }
	  strncpy(temp, key.value[i], key.len_value[i]);
          temp[key.len_value[i]] = '\0';

	  this->file_name[i] = DupString(temp);
          if (this->file_name[i] == (char *)NULL) {
            error_string = "converting file name string";
	    break;
	  }
	}
	if (error_string != (char *)NULL) break;
        nband_file_name = key.nval;
        break;

/******************************************************************/
/*********************** get thermal values ***********************/
/******************************************************************/

      case HEADER_NSAMPLE_TH:
        if (key.nval <= 0) {
	  error_string = "no number of thermal sample value";
	  break; 
	} else if (key.nval > 1) {
	  error_string = "too many number of thermal sample values";
	  break; 
	}
        if (key.len_value[0] < 1) {
	  error_string = "invalid number of thermal samples string";
	  break;
	}
	strncpy(temp, key.value[0], key.len_value[0]);
        temp[key.len_value[0]] = '\0';

	if (sscanf(temp, "%d", &this->size_th.s) != 1) {
	  error_string = "converting number of thermal samples";
	  break;
	}
	if (this->size_th.s < 1  ||
	    this->size_th.s > NSAMP_MAX)
	  error_string = "thermal sample number out of range";
        break;

      case HEADER_NLINE_TH:
        if (key.nval <= 0) {
	  error_string = "no number of thermal line value";
	  break; 
	} else if (key.nval > 1) {
	  error_string = "too many number of thremal line values";
	  break; 
	}
        if (key.len_value[0] < 1) {
	  error_string = "invalid number of thermal lines string";
	  break;
	}
	strncpy(temp, key.value[0], key.len_value[0]);
        temp[key.len_value[0]] = '\0';

	if (sscanf(temp, "%d", &this->size_th.l) != 1) {
	  error_string = "converting number of thremal lines";
	  break;
	}
	if (this->size_th.l < 1  ||
	    this->size_th.l > NLINE_MAX)
	  error_string = "thremal line number out of range";
        break;

      case HEADER_FILES_TH:
        if (key.nval <= 0) {
	  error_string = "no thermal file name value";
	  break; 
	} else if (key.nval > 1) {
	  error_string = "too many thermal file name values";
	  break; 
	}
          if (key.len_value[0] < 1) {
	    error_string = "invalid thermal file name string";
	    break;
	  }
	  strncpy(temp, key.value[0], key.len_value[0]);
          temp[key.len_value[0]] = '\0';

	  this->file_name_th = DupString(temp);
          if (this->file_name_th == (char *)NULL) {
            error_string = "converting thermal file name string";
	    break;
	  }

	if (error_string != (char *)NULL) break;
        break;

      case HEADER_NBAND_TH:
        if (key.nval <= 0) {
	  error_string = "no number of thermal bands value";
	  break; 
	} else if (key.nval > 1) {
	  error_string = "too many number of thermal bands values";
	  break; 
	}
        if (key.len_value[0] < 1) {
	  error_string = "invalid number of thermal bands string";
	  break;
	}
	strncpy(temp, key.value[0], key.len_value[0]);
        temp[key.len_value[0]] = '\0';

	if (sscanf(temp, "%d", &this->nband_th) != 1) {
	  error_string = "converting number of thermal bands";
	  break;
	}
	if (this->nband < 1  ||
	    this->nband > NBAND_REFL_MAX)
	  error_string = "number of thermal bands out of range";
        break;

      case HEADER_BANDS_TH:
        if (key.nval <= 0) {
	  error_string = "no thermal band number value";
	  break; 
	} else if (key.nval > 1) {
	  error_string = "too many thermal band number values";
	  break; 
	}
          if (key.len_value[0] < 1) {
	    error_string = "invalid thermal band number string";
	    break;
	  }
	  strncpy(temp, key.value[0], key.len_value[0]);
	  temp[key.len_value[0]] = '\0';

	  if (sscanf(temp, "%d", &this->meta.iband_th) != 1) {
	    error_string = "converting thermal band number";
	    break;
	  }
	  if (this->meta.iband_th < 1) {
	    error_string = "thermal band number out of range";
	    break;
	  }

	if (error_string != (char *)NULL) break;
        break;

      case HEADER_GAIN_SETTINGS_TH:
        if (key.nval <= 0) {
	  error_string = "no thermal gain values";
	  break; 
	} else if (key.nval > 1) {
	  error_string = "too many GAIN SETTING TH values";
	  break; 
	}

          if (key.len_value[0] < 1) {
	    error_string = "invalid thermal gain setting string";
	    break;
	  }
	  strncpy(temp, key.value[0], key.len_value[0]);
          temp[key.len_value[0]] = '\0';

          this->meta.gain_setting_th = (Gain_t)
	    KeyString(key.value[0], key.len_value[0],
	              Gain_string, (int)GAIN_NULL, (int)GAIN_MAX);
		      		      
	if (error_string != (char *)NULL) break;
        break;
	
      case HEADER_GAIN_TH:
        if (key.nval <= 0) {
	  error_string = "no thermal gain values";
	  break; 
	} else if (key.nval > 1) {
	  error_string = "too many thermal gain values";
	  break; 
	}

          if (key.len_value[0] < 1) {
	    error_string = "invalid thermal gain string";
	    break;
	  }
	  strncpy(temp, key.value[0], key.len_value[0]);
          temp[key.len_value[0]] = '\0';

/*        printf("gain_th string=(%s)\n",temp); */

	  if (sscanf(temp, "%g", &this->meta.gain_th) != 1) {
	    error_string = "converting thermal gain";
	    break;
	  }

	if (error_string != (char *)NULL) break;
        break;

      case HEADER_BIAS_TH:
        if (key.nval <= 0) {
	  error_string = "no bais values";
	  break; 
	} else if (key.nval > NBAND_REFL_MAX) {
	  error_string = "too many thermal bias values";
	  break; 
	}

          if (key.len_value[0] < 1) {
	    error_string = "invalid thermal bias string";
	    break;
	  }
	  strncpy(temp, key.value[0], key.len_value[0]);
          temp[key.len_value[0]] = '\0';

/*        printf("bias_th string=(%s)\n",temp); */
	  
	  if (sscanf(temp, "%g", &this->meta.bias_th) != 1) {
	    error_string = "converting thermal bias";
	    break;
	  }

	if (error_string != (char *)NULL) break;
        break;

/******************************************************************/
/******************** end of get thermal values *******************/
/******************************************************************/
      case HEADER_END:
        if (key.nval != 0) {
	  error_string = "no value expected";
	  break; 
	}
        got_end = true;
        break;

      default:
	/*
        error_string = "key not implmented";
	*/
	break;
    }
    if (error_string != (char *)NULL) break;
    if (got_end) break;
  }

  /* Handle errors */

  if (error_string != (char *)NULL) {
    for (ib = 0; ib < NBAND_REFL_MAX; ib++)
      if (this->file_name[ib] != (char *)NULL) free(this->file_name[ib]);
    free(this->file_header_name);
    RETURN_ERROR(error_string, "GetHeaderInput", false);
  }

  /* Make sure both the start and end of file exist */
  
  if (!got_start) 
    error_string = "no start key in header";
  else if (!got_end)
    error_string = "no end key in header";

  if (error_string != (char *)NULL) {
    for (ib = 0; ib < NBAND_REFL_MAX; ib++)
      if (this->file_name[ib] != (char *)NULL) free(this->file_name[ib]);
    free(this->file_header_name);
    RETURN_ERROR(error_string, "GetHeaderInput", false);
  }

  /* Handle null values */
  
  if (this->file_type == INPUT_TYPE_NULL) 
    error_string = "no file type given";
  else if (this->meta.provider == PROVIDER_NULL) 
    error_string = "no data provider given";
  else if (this->meta.sat == SAT_NULL) 
    error_string = "no satellite given";
  else if (this->meta.inst == INST_NULL) 
    error_string = "no instrument type given";
  else if (acq_date[0] == '\0') 
    error_string = "no acquisition date given";
  else if (this->meta.inst == INST_ETM  &&  prod_date[0] == '\0') 
    error_string = "no production date given";
  else if (this->meta.sun_zen == ANGLE_FILL) 
    error_string = "no solar zenith angle given";
  else if (this->meta.sun_az == ANGLE_FILL) 
    error_string = "no solar azimuth angle given";
  else if (this->meta.wrs_sys == WRS_FILL) 
    error_string = "no WRS system given";
  else if (this->meta.ipath == WRS_FILL) 
    error_string = "no number of paths given";
  else if (this->meta.irow == WRS_FILL) 
    error_string = "no number of rows given";
  else if (this->nband < 1) 
    error_string = "no number of bands given";
  else if (this->size.s < 1) 
    error_string = "no number of samples given";
  else if (this->size.l < 1) 
    error_string = "no number of lines given";
  else if (this->nband != nband_iband)
    error_string = "inconsistant number of values (band numbers)";
  else if (flag_gain_set  &&  this->nband != nband_gain_set)
    error_string = "inconsistant number of values (gain settings)";
  else if (flag_gain  &&  this->nband != nband_gain)
    error_string = "inconsistant number of values (gains)";
  else if (flag_bias  &&  this->nband != nband_bias)
    error_string = "inconsistant number of values (biases)";
  else if (this->nband != nband_file_name)
    error_string = "inconsistant number of values (file names)";

  if (error_string != (char *)NULL) {
    for (ib = 0; ib < NBAND_REFL_MAX; ib++)
      if (this->file_name[ib] != (char *)NULL) free(this->file_name[ib]);
    free(this->file_header_name);
    RETURN_ERROR(error_string, "GetHeaderInput", false);
  }

  for (ib = 0; ib < this->nband;    ib++) {
    if (this->meta.iband[ib] < 1) {
      error_string = "no band number given";
      break;
    }
    if (this->meta.inst == INST_ETM && !param->ETM_GB) {
      if (this->meta.gain_set[ib] == GAIN_NULL) {
        error_string = "no gain setting given";
        break;
      }
      if (this->meta.gain[ib] != GAIN_BIAS_FILL) {
        error_string = "no gain allowed";
        break;
      }
      if (this->meta.bias[ib] != GAIN_BIAS_FILL) {
        error_string = "no bias allowed";
        break;
      }
    } else if (!param->ETM_GB) {
      if (this->meta.gain_set[ib] != GAIN_NULL) {
        error_string = "no gain setting allowed";
        break;
      }
      if (this->meta.gain[ib] == GAIN_BIAS_FILL) {
        error_string = "no gain given";
        break;
      }
      if (this->meta.bias[ib] == GAIN_BIAS_FILL) {
        error_string = "no bias given";
        break;
      }
    }
    if (this->meta.inst == INST_ETM && param->ETM_GB) {
    this->meta.gain_th= this->meta.gain_th;
    }
    
    if (this->file_name[ib] == (char *)NULL) {
      error_string = "no file name given";
      break;
    }
  }
  
  if (error_string != (char *)NULL) {
    for (ib = 0; ib < this->nband;    ib++)
      if (this->file_name[ib] != (char *)NULL) free(this->file_name[ib]);
    free(this->file_header_name);
    RETURN_ERROR(error_string, "GetHeaderInput", false);
  }

  /* Check WRS path/rows */
  
  if (this->meta.wrs_sys == WRS_1) {
    if (this->meta.ipath > 251)
      error_string = "WRS path number out of range";
    else if (this->meta.irow > 248)
      error_string = "WRS row number out of range";
  } else if (this->meta.wrs_sys == WRS_2) {
    if (this->meta.ipath > 233)
      error_string = "WRS path number out of range";
    else if (this->meta.irow > 248)
      error_string = "WRS row number out of range";
  } else
    error_string = "invalid WRS system";

  if (error_string != (char *)NULL) {
    for (ib = 0; ib < this->nband;    ib++)
      if (this->file_name[ib] != (char *)NULL) free(this->file_name[ib]);
    free(this->file_header_name);
    RETURN_ERROR(error_string, "GetHeaderInput", false);
  }

  /* Check satellite/instrument combination */
  
  if (this->meta.inst == INST_MSS) {
    if (this->meta.sat != SAT_LANDSAT_1  &&  
        this->meta.sat != SAT_LANDSAT_2  &&
        this->meta.sat != SAT_LANDSAT_3  &&  
	this->meta.sat != SAT_LANDSAT_4  &&
        this->meta.sat != SAT_LANDSAT_5)
      error_string = "invalid insturment/satellite combination";
  } else if (this->meta.inst == INST_TM) {
    if (this->meta.sat != SAT_LANDSAT_4  &&  
        this->meta.sat != SAT_LANDSAT_5)
      error_string = "invalid insturment/satellite combination";
  } else if (this->meta.inst == INST_ETM) {
    if (this->meta.sat != SAT_LANDSAT_7)
      error_string = "invalid insturment/satellite combination";
  } else
    error_string = "invalid instrument type";

  if (error_string != (char *)NULL) {
    for (ib = 0; ib < this->nband;    ib++) 
      if (this->file_name[ib] != (char *)NULL) free(this->file_name[ib]);
    free(this->file_header_name);
    RETURN_ERROR(error_string, "GetHeaderInput", false);
  }

  /* Check band numbers */

  if (this->meta.inst == INST_MSS) {
    for (ib = 0; ib < this->nband; ib++)
      if (this->meta.iband[ib] > 4) {
        error_string = "band number out of range";
	break;
      }
  } else if (this->meta.inst == INST_TM  ||
             this->meta.inst == INST_ETM) {
    for (ib = 0; ib < this->nband; ib++)
      if (this->meta.iband[ib] == 6  ||  this->meta.iband[ib] > 7) {
        error_string = "band number out of range";
	break;
      }
  }
  
  if (error_string != (char *)NULL) {
    for (ib = 0; ib < this->nband;    ib++)
      if (this->file_name[ib] != (char *)NULL) free(this->file_name[ib]);
    free(this->file_header_name);
    RETURN_ERROR(error_string, "GetHeaderInput", false);
  }

  /* Convert the date/time values */

  if (acq_time[0] != '\0') 
    this->meta.time_fill = false;
  else
    strcpy(acq_time, "00:00:00");
    
  if (sprintf(temp, "%sT%sZ", acq_date, acq_time) < 0) 
    error_string = "converting acquisition date/time";
  else if (!DateInit(&this->meta.acq_date, temp, DATE_FORMAT_DATEA_TIME))
    error_string = "converting acquisition date/time";

  if (error_string == (char *)NULL  &&  
      prod_date[0] != '\0') {
    if (!DateInit(&this->meta.prod_date, prod_date, DATE_FORMAT_DATEA))
      error_string = "converting production date/time";
  }

  if (error_string != (char *)NULL) {
    for (ib = 0; ib < this->nband;    ib++)
      if (this->file_name[ib] != (char *)NULL) free(this->file_name[ib]);
    free(this->file_header_name);
    RETURN_ERROR(error_string, "GetHeaderInput", false);
  }

  return true;
}

#endif
