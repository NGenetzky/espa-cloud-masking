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
#define INPUT_REFL_GAINS ("ReflGains")
#define INPUT_REFL_BIAS ("ReflBias")
#define INPUT_TH_GAIN ("ThermalGain")
#define INPUT_TH_BIAS ("ThermalBias")
#define INPUT_FILL_VALUE ("_FillValue")
#define INPUT_WEST_BOUND  ("WestBoundingCoordinate")
#define INPUT_EAST_BOUND  ("EastBoundingCoordinate")
#define INPUT_NORTH_BOUND ("NorthBoundingCoordinate")
#define INPUT_SOUTH_BOUND ("SouthBoundingCoordinate")
#define INPUT_UL_LAT_LONG ("UpperLeftCornerLatLong")
#define INPUT_LR_LAT_LONG ("LowerRightCornerLatLong")

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
#define PI (3.141592653589793238)


/******************************************************************************
MODULE:  trim_white_space

PURPOSE:  Trim the white space before the string

RETURN: trimed string

HISTORY:
Date        Programmer       Reason
--------    ---------------  ----------------------------------------------
3/15/2013   Song Guo         Original Development (based on online example)

NOTES: 
*****************************************************************************/ 
char *trim_white_space(char *str)
{
    char *end; /* end character */

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

NOTES: The constants and formular used are from BU's matlab code
       & G. Chander et al. RSE 113 (2009) 893-903
*****************************************************************************/ 
bool dn_to_bt(Input_t *input)
{
    float k1, k2; /* constans */
    int dn = 255; /* maximum DN value */
    float temp;   /* intermediate variable */
    char errmsg[STR_SIZE];   /* error message */

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
        sprintf (errmsg, "Unsupported satellite sensor");
        RETURN_ERROR(errmsg, "dn_to_bt", false);
    }

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

NOTES: The constants and formular used are from BU's matlab code
       & G. Chander et al. RSE 113 (2009) 893-903  
******************************************************************************/
bool dn_to_toa(Input_t *input)
{
    float esun[6]; /* earth sun distance for each band */
    int ib;        /* band loop variable */
    int dn = 255;  /* maximum DN value */
    float temp;    /* intermediate variable */
    char errmsg[STR_SIZE];   /* error message */

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
        sprintf (errmsg, "Unsupported satellite sensor");
        RETURN_ERROR(errmsg, "dn_to_bt", false);
    }

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
!Description: 'OpenInput' sets up the 'input' data structure, opens the
 input file for read access, allocates space, and stores some of the metadata.
 
!Input Parameters:
 file_name      input file name

!Output Parameters:
 (returns)      populated 'input' data structure or NULL when an error occurs

!Team Unique Header:

!Design Notes:
******************************************************************************/
Input_t *OpenInput(char *lndth_name, char *lndcal_name)
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
  FILE *in;
  int i;  
  char *path = NULL;
  char full_path[MAX_STR_LEN];
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
  this->sds_th_file_id = SDstart((char *)lndth_name, DFACC_RDONLY);
  if (this->sds_th_file_id == HDF_ERROR) {
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

  if (!GetSDSInfo(this->sds_th_file_id, &this->therm_sds))
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

    path = getenv("ESUN");
    if (path == NULL)
    {
        error_string = "ESUN environment variable is not set";
        ERROR(error_string, "OpenInput");
    }

    sprintf(full_path,"%s/%s",path,"EarthSunDistance.txt");
    in = fopen(full_path, "r");
    if (in == NULL)
    {
        error_string = "Can't open EarthSunDistance.txt file";
        ERROR(error_string, "OpenInput");
    }

    for (i = 0; i < 366; i++)
    {
        if (fscanf(in, "%f", &this->dsun_doy[i]) == EOF)
        {
            error_string = "End of file (EOF) is met before 336 lines";
            ERROR(error_string, "OpenInput");
        }            
    }
    fclose(in);

    /* Calculate maximum TOA reflectance values and put them in metadata */
    status = dn_to_toa(this);
    if (!status)
    {
        error_string = "Error calling dn_to_toa routine";
        ERROR(error_string, "OpenInput");
    }

    /* Calculate maximum BT values and put them in metadata */
    status = dn_to_bt(this);
    if (!status)
    {
        error_string = "Error calling dn_to_bt routine";
        ERROR(error_string, "OpenInput");
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
  SDend(this->sds_th_file_id);
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
                  'true' = okay
                  'false' = error return

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
  if (!GetAttrString(this->sds_th_file_id, &attr, this->meta.provider))
    RETURN_ERROR("reading attribute (data provider)", "GetInputMeta", false);

  attr.type = DFNT_CHAR8;
  attr.nval = MAX_STR_LEN;
  attr.name = INPUT_SAT;
  if (!GetAttrString(this->sds_th_file_id, &attr, this->meta.sat))
    RETURN_ERROR("reading attribute (instrument)", "GetInputMeta", false);

  attr.type = DFNT_CHAR8;
  attr.nval = MAX_STR_LEN;
  attr.name = INPUT_INST;
  if (!GetAttrString(this->sds_th_file_id, &attr, this->meta.inst))
    RETURN_ERROR("reading attribute (instrument)", "GetInputMeta", false);

  attr.type = DFNT_CHAR8;
  attr.nval = MAX_DATE_LEN;
  attr.name = INPUT_ACQ_DATE;
  if (!GetAttrString(this->sds_th_file_id, &attr, date))
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
  if (!GetAttrDouble(this->sds_th_file_id, &attr, dval))
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
  if (!GetAttrDouble(this->sds_th_file_id, &attr, dval))
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
  if (!GetAttrString(this->sds_th_file_id, &attr, this->meta.wrs_sys))
    RETURN_ERROR("reading attribute (WRS system)", "GetInputMeta", false);

  attr.type = DFNT_INT16;
  attr.nval = 1;
  attr.name = INPUT_WRS_PATH;
  if (!GetAttrDouble(this->sds_th_file_id, &attr, dval))
    RETURN_ERROR("reading attribute (WRS path)", "GetInputMeta", false);
  if (attr.nval != 1) 
    RETURN_ERROR("invalid number of values (WRS path)", "GetInputMeta", false);
  meta->path = (int)floor(dval[0] + 0.5);
  if (meta->path < 1) 
    RETURN_ERROR("WRS path out of range", "GetInputMeta", false);

  attr.type = DFNT_INT16;
  attr.nval = 1;
  attr.name = INPUT_WRS_ROW;
  if (!GetAttrDouble(this->sds_th_file_id, &attr, dval))
    RETURN_ERROR("reading attribute (WRS row)", "GetInputMeta", false);
  if (attr.nval != 1) 
    RETURN_ERROR("invalid number of values (WRS row)", "GetInputMeta", false);
  meta->row = (int)floor(dval[0] + 0.5);
  if (meta->row < 1) 
    RETURN_ERROR("WRS path out of range", "GetInputMeta", false);

    /* Get the upper left and lower right corners */
    meta->ul_corner.is_fill = false;
    attr.type = DFNT_FLOAT32;
    attr.nval = 2;
    attr.name = INPUT_UL_LAT_LONG;
    if (GetAttrDouble(this->sds_cal_file_id, &attr, dval) != SUCCESS)
    {
        printf ("Unable to read the UL lat/long coordinates.\n");
        meta->ul_corner.is_fill = true;
    }
    if (attr.nval != 2) 
    {
        RETURN_ERROR("Invalid number of values for the UL lat/long",
                     "GetInputMeta", false);
    }
    meta->ul_corner.lat = dval[0];
    meta->ul_corner.lon = dval[1];

    meta->lr_corner.is_fill = false;
    attr.type = DFNT_FLOAT32;
    attr.nval = 2;
    attr.name = INPUT_LR_LAT_LONG;
    if (GetAttrDouble(this->sds_cal_file_id, &attr, dval) != SUCCESS)
    {
        printf ("Unable to read the LR lat/long coordinates.\n");
        meta->lr_corner.is_fill = true;
    }
    if (attr.nval != 2) 
    {
        RETURN_ERROR("Invalid number of values for the LR lat/long",
                     "GetInputMeta", false);
    }
    meta->lr_corner.lat = dval[0];
    meta->lr_corner.lon = dval[1];

    /* Get the bounding coordinates if they are available */
    meta->bounds.is_fill = false;
    attr.type = DFNT_FLOAT32;
    attr.nval = 1;
    attr.name = INPUT_WEST_BOUND;
    if (!GetAttrDouble(this->sds_cal_file_id, &attr, dval))
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
    if (!GetAttrDouble(this->sds_cal_file_id, &attr, dval))
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
    if (!GetAttrDouble(this->sds_cal_file_id, &attr, dval))
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
    if (!GetAttrDouble(this->sds_cal_file_id, &attr, dval))
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

    /* Read the reflectance band gains */
    attr.type = DFNT_FLOAT64;
    attr.nval = this->nband;
    attr.name = INPUT_REFL_GAINS;
    if (!GetAttrDouble(this->sds_cal_file_id, &attr, dval))
      RETURN_ERROR("reading attribute (reflectance gains)", "GetInputMeta",
          false);
    if (attr.nval != this->nband) 
      RETURN_ERROR("invalid number of values (reflectance gains)", 
                   "GetInputMeta", false);
    for (ib = 0; ib < this->nband; ib++)
      meta->gain[ib] = (float) dval[ib];

    /* Read the reflectance band biases */
    attr.type = DFNT_FLOAT64;
    attr.nval = this->nband;
    attr.name = INPUT_REFL_BIAS;
    if (!GetAttrDouble(this->sds_cal_file_id, &attr, dval))
      RETURN_ERROR("reading attribute (reflectance biases)", "GetInputMeta",
          false);
    if (attr.nval != this->nband) 
      RETURN_ERROR("invalid number of values (reflectance biases)", 
                   "GetInputMeta", false);
    for (ib = 0; ib < this->nband; ib++)
      meta->bias[ib] = (float) dval[ib];

    /* Read the thermal band gain */
    attr.type = DFNT_FLOAT64;
    attr.nval = 1;
    attr.name = INPUT_TH_GAIN;
    if (!GetAttrDouble(this->sds_th_file_id, &attr, dval))
      RETURN_ERROR("reading attribute (reflectance gains)", "GetInputMeta",
        false);
    meta->gain_th = (float) dval[0];

    /* Read the thermal band bias */
    attr.type = DFNT_FLOAT64;
    attr.nval = 1;
    attr.name = INPUT_TH_BIAS;
    if (!GetAttrDouble(this->sds_th_file_id, &attr, dval))
      RETURN_ERROR("reading attribute (thermal bias)", "GetInputMeta",
        false);
    meta->bias_th = (float) dval[0];

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
    RETURN_ERROR("reading attribute (unit ref)", "GetInputMeta2", false);

  attr.type = DFNT_INT16;
  attr.nval = 2;
  attr.name = INPUT_VALID_RANGE;
  if (!GetAttrDouble(this->sds[0].id, &attr, dval))
    RETURN_ERROR("reading attribute (valid range ref)", "GetInputMeta2", false);
  if (attr.nval != 2) 
    RETURN_ERROR("invalid number of values (valid range ref)", "GetInputMeta2", false);
  this->meta.valid_range_ref[0] = (float)dval[0];
  this->meta.valid_range_ref[1] = (float)dval[1];

  for (ib = 0; ib < this->nband; ib++) {
      attr.type = DFNT_INT16;
      attr.nval = 1;
      attr.name = INPUT_SATU_VALUE;
      if (!GetAttrDouble(this->sds[ib].id, &attr, dval))
         RETURN_ERROR("reading attribute (saturate value ref)", "GetInputMeta2",
                     false);
      if (attr.nval != 1) 
         RETURN_ERROR("invalid number of values (saturate value ref)", 
              "GetInputMeta2", false);
       this->meta.satu_value_ref[ib] = (int)dval[0];
  }

  attr.type = DFNT_INT16;
  attr.nval = 1;
  attr.name = INPUT_SATU_VALUE;
  if (!GetAttrDouble(this->sds[0].id, &attr, dval))
     RETURN_ERROR("reading attribute (saturate value ref)", "GetInputMeta2",
                  false);
  if (attr.nval != 1) 
     RETURN_ERROR("invalid number of values (saturate value ref)", 
              "GetInputMeta2", false);
  this->meta.therm_satu_value_ref = (int)dval[0];

  attr.type = DFNT_FLOAT64;
  attr.nval = 1;
  attr.name = INPUT_SCALE_FACTOR;
  if (!GetAttrDouble(this->sds[0].id, &attr, dval))
    RETURN_ERROR("reading attribute (scale factor ref)", "GetInputMeta2", false);
  if (attr.nval != 1) 
    RETURN_ERROR("invalid number of values (scale factor ref)", "GetInputMeta2", false);
  this->meta.scale_factor_ref = (float)dval[0];

  attr.type = DFNT_FLOAT64;
  attr.nval = 1;
  attr.name = INPUT_ADD_OFFSET;
  if (!GetAttrDouble(this->sds[0].id, &attr, dval))
    RETURN_ERROR("reading attribute (add offset ref)", "GetInputMeta2", false);
  if (attr.nval != 1) 
    RETURN_ERROR("invalid number of values (add offset ref)", "GetInputMeta2", false);
  this->meta.add_offset_ref = (float)dval[0];

  attr.type = DFNT_FLOAT64;
  attr.nval = 1;
  attr.name = INPUT_SCALE_FACTOR_ERR;
  if (!GetAttrDouble(this->sds[0].id, &attr, dval))
    RETURN_ERROR("reading attribute (scale factor err  ref)", "GetInputMeta2", false);
  if (attr.nval != 1) 
    RETURN_ERROR("invalid number of values (scale factor err ref)", "GetInputMeta2", false);
  this->meta.scale_factor_err_ref = (float)dval[0];

  attr.type = DFNT_FLOAT64;
  attr.nval = 1;
  attr.name = INPUT_ADD_OFFSET_ERR;
  if (!GetAttrDouble(this->sds[0].id, &attr, dval))
    RETURN_ERROR("reading attribute (add offset err ref)", "GetInputMeta2", false);
  if (attr.nval != 1) 
    RETURN_ERROR("invalid number of values (add offset err ref)", "GetInputMeta2", false);
  this->meta.add_offset_err_ref = (float)dval[0];

  attr.type = DFNT_FLOAT32;
  attr.nval = 1;
  attr.name = INPUT_CALIBRATED_NT;
  if (!GetAttrDouble(this->sds[0].id, &attr, dval))
    RETURN_ERROR("reading attribute (calibrated NT ref)", "GetInputMeta2", false);
  if (attr.nval != 1) 
    RETURN_ERROR("invalid number of values (calibrated NT ref)", "GetInputMeta2", false);
  this->meta.calibrated_nt_ref = (float)dval[0];

  return true;
}

