/*****************************************************************************
!File: output.c
  
!Description: Functions creating and writting data to the product output file.

!Revision History:
 Song Guo 
 Original Version - borrowed and modified from some of the LEDAPS libraries.

!Team Unique Header:
  This software was developed by the Landsat Science, Research, and
  Development (LSRD) Team at the USGS EROS.

!Design Notes:
 1. The following public functions handle the output files:
    CreateOutput - Create new output file.
    OutputFile - Setup 'output' data structure.
    CloseOutput - Close the output file.
    FreeOutput - Free the 'output' data structure memory.
    PutMetadata - Write the output product metadata.
    WriteOutput - Write a line of data to the output product file.
*****************************************************************************/

#include "output.h"

#define SDS_PREFIX ("band")

#define OUTPUT_PROVIDER ("DataProvider")
#define OUTPUT_SAT ("Satellite")
#define OUTPUT_INST ("Instrument")
#define OUTPUT_ACQ_DATE ("AcquisitionDate")
#define OUTPUT_L1_PROD_DATE ("Level1ProductionDate")
#define OUTPUT_SUN_ZEN ("SolarZenith")
#define OUTPUT_SUN_AZ ("SolarAzimuth")
#define OUTPUT_WRS_SYS ("WRS_System")
#define OUTPUT_WRS_PATH ("WRS_Path")
#define OUTPUT_WRS_ROW ("WRS_Row")
#define OUTPUT_NBAND ("NumberOfBands")
#define OUTPUT_BANDS ("BandNumbers")
#define OUTPUT_SHORT_NAME ("ShortName")
#define OUTPUT_LOCAL_GRAN_ID ("LocalGranuleID")
#define OUTPUT_PROD_DATE ("ProductionDate")
#define OUTPUT_CFMASK_VERSION ("CFmaskVersion")

#define OUTPUT_WEST_BOUND  ("WestBoundingCoordinate")
#define OUTPUT_EAST_BOUND  ("EastBoundingCoordinate")
#define OUTPUT_NORTH_BOUND ("NorthBoundingCoordinate")
#define OUTPUT_SOUTH_BOUND ("SouthBoundingCoordinate")
#define OUTPUT_UL_LAT_LONG ("UpperLeftCornerLatLong")
#define OUTPUT_LR_LAT_LONG ("LowerRightCornerLatLong")

#define OUTPUT_LONG_NAME        ("long_name")
#define OUTPUT_UNITS            ("units")
#define OUTPUT_VALID_RANGE      ("valid_range")
#define OUTPUT_FILL_VALUE       ("_FillValue")
#define OUTPUT_SATU_VALUE       ("_SaturateValue")
#define OUTPUT_SCALE_FACTOR     ("scale_factor")
#define OUTPUT_ADD_OFFSET       ("add_offset")
#define OUTPUT_SCALE_FACTOR_ERR ("scale_factor_err")
#define OUTPUT_ADD_OFFSET_ERR   ("add_offset_err")
#define OUTPUT_CALIBRATED_NT    ("calibrated_nt")
#define OUTPUT_QAMAP_INDEX      ("qa_bitmap_index")
#define OUTPUT_MASK_INDEX       ("mask_index")

#define OUTPUT_DATA_FIELD_NAME ("DataFieldName")
#define OUTPUT_DATA_TYPE ("DataType")
#define OUTPUT_DIM_LIST ("DimList")

#define CFMASK_VERSION "1.2.1"
#define FMASK_BAND "fmask_band"
#define DATA_TYPE "DFNT_UINT8"
#define DIM_LIST "YDIM,XDIM"
#define LONG_NAME "fmask_band"
#define MASK_INDEX "0 clear; 1 water; 2 cloud_shadow; 3 snow; 4 cloud"
#define PI (3.141592653589793238)

/******************************************************************************
!Description: 'CreateOuptut' creates a new output file.
 
!Input Parameters:
 file_name      output file name

!Output Parameters:
 (returns)      status:
                  'true' = okay
                  'false' = error return

!Team Unique Header:

!Design Notes:
*****************************************************************************/
bool CreateOutput(char *file_name)
{
  int32 hdf_file_id;

  /* Create the file with HDF open */
  hdf_file_id = Hopen(file_name, DFACC_CREATE, DEF_NDDS); 
  if (hdf_file_id == HDF_ERROR) {
    RETURN_ERROR("creating output file", "CreateOutput", false); 
  }

  /* Close the file */
  Hclose(hdf_file_id);

  return true;
}

/******************************************************************************
!Description: 'OutputFile' sets up the 'output' data structure, opens the
 output file for write access, and creates the output Science Data Set (SDS).
 
!Input Parameters:this
 file_name      output file name
 nband          number of image bands (SDSs) to be created
 nband_qa       number of QA bands (SDSs) to be created
 sds_names      array of SDS names for each image band
 qa_sds_names   array of SDS names for each QA band
 size           SDS size

!Output Parameters:
 (returns)      'output' data structure or NULL when an error occurs

!Team Unique Header:

!Design Notes:
*****************************************************************************/
Output_t *OpenOutput(char *file_name, char sds_names[MAX_STR_LEN],
   Img_coord_int_t *size)
{
  Output_t *this = NULL;
  char *error_string = NULL;
  Myhdf_dim_t *dim[MYHDF_MAX_RANK];
  Myhdf_sds_t *sds = NULL;
  int ir;    /* looping variable for rank/dimension */
  int ib;    /* looping variable for bands */

  /* Check parameters */
  if (size->l < 1)
    RETURN_ERROR("invalid number of output lines", "OpenOutput", NULL);

  if (size->s < 1)
    RETURN_ERROR("invalid number of samples per output line", "OpenOutput",
      NULL);

  /* Create the Output data structure */
  this = (Output_t *)malloc(sizeof(Output_t));
  if (this == NULL) 
    RETURN_ERROR("allocating Output data structure", "OpenOutput", NULL);

  /* Populate the data structure */
  this->file_name = DupString(file_name);
  if (this->file_name == NULL) {
    free(this);
    RETURN_ERROR("duplicating file name", "OpenOutput", NULL);
  }

  this->open = false;
  this->nband = 1;
  this->size.l = size->l;
  this->size.s = size->s;
  for (ib = 0; ib < this->nband; ib++)
  {
    this->sds[ib].name = NULL;
    this->sds[ib].dim[0].name = NULL;
    this->sds[ib].dim[1].name = NULL;
  }

  /* Open file for SD access */
  this->sds_file_id = SDstart((char *)file_name, DFACC_RDWR);
  if (this->sds_file_id == HDF_ERROR) {
    free(this->file_name);
    free(this);  
    RETURN_ERROR("opening output file for SD access", "OpenOutput", NULL); 
  }
  this->open = true;

  /* Set up the image SDSs */
  for (ib = 0; ib < this->nband; ib++)
  {
      sds = &this->sds[ib];
      sds->rank = 2;
      sds->type = DFNT_UINT8;
      sds->name = DupString(sds_names);
      if (sds->name == NULL) {
          error_string = "duplicating sds name";
          RETURN_ERROR(error_string, "CreateOutput", NULL); 
      }

     dim[0] = &sds->dim[0];
     dim[1] = &sds->dim[1];

     dim[0]->nval = this->size.l;
     dim[1]->nval = this->size.s;

     dim[0]->type = dim[1]->type = sds->type;

     dim[0]->name = DupString("YDim_Grid");
     if (dim[0]->name == NULL) {
          error_string = "duplicating dim name (l)";
          RETURN_ERROR(error_string, "CreateOutput", NULL); 
     }
     dim[1]->name = DupString("XDim_Grid");
     if (dim[1]->name == NULL) {
         error_string = "duplicating dim name (s)";
         RETURN_ERROR(error_string, "CreateOutput", NULL); 
    }

    if (!PutSDSInfo(this->sds_file_id, sds)) {
      error_string = "setting up the SDS";
      RETURN_ERROR(error_string, "CreateOutput", NULL); 
    }

    for (ir = 0; ir < sds->rank; ir++) {
      if (!PutSDSDimInfo(sds->id, dim[ir], ir)) {
        error_string = "setting up the dimension";
        RETURN_ERROR(error_string, "CreateOutput", NULL); 
      }
    }
  }

  return this;
}


/******************************************************************************
!Description: 'CloseOutput' ends SDS access and closes the output file.
 
!Input Parameters:
 this           'output' data structure

!Output Parameters:
 this           'output' data structure
 (returns)      status:
                  'true' = okay
                  'false' = error return

!Team Unique Header:

!Design Notes:
*****************************************************************************/
bool CloseOutput(Output_t *this)
{
  int ib;

  if (!this->open)
    RETURN_ERROR("file not open", "CloseOutput", false);

  /* Close image SDSs */
  for (ib = 0; ib < this->nband; ib++)
  {
    if (SDendaccess(this->sds[ib].id) == HDF_ERROR) 
      RETURN_ERROR("ending sds access", "CloseOutput", false);
  }

  /* Close the HDF file itself */
  SDend(this->sds_file_id);
  this->open = false;

  return true;
}


/******************************************************************************
!Description: 'FreeOutput' frees the 'output' data structure memory.
 
!Input Parameters:
 this           'output' data structure

!Output Parameters:
 this           'output' data structure
 (returns)      status:
                  'true' = okay (always returned)

!Team Unique Header:

!Design Notes:
*****************************************************************************/
bool FreeOutput(Output_t *this)
{
  int ir;    /* looping variable for rank/dimension */
  int ib;    /* looping variable for bands */

  if (this->open) 
    RETURN_ERROR("file still open", "FreeOutput", false);

  if (this != NULL) {
    /* Free image band SDSs */
        /* Free image band SDSs */
        for (ib = 0; ib < this->nband; ib++){
            for (ir = 0; ir < this->sds[ib].rank; ir++) {
                if (this->sds[ib].dim[ir].name != NULL) 
                     free(this->sds[ib].dim[ir].name);
            }
        if (this->sds[ib].name != NULL) 
             free(this->sds[ib].name);
        }

        if (this->file_name != NULL)
            free(this->file_name);
        free(this);
  }

  return true;
}

/******************************************************************************
!Description: 'PutOutputLine' writes nlines of data to the output file.
 
!Input Parameters:
 this           'output' data structure; the following fields are written:
                buf -- contains the line to be written
 final_mask     current nlines of data to be written (0-based)

!Output Parameters:
 this           'output' data structure; the following fields are modified:
 (returns)      status:
                  'true' = okay
                  'false' = error return

!Team Unique Header:

!Design Notes:
*****************************************************************************/
bool PutOutput(Output_t *this, unsigned char **final_mask)
{
  int32 start[MYHDF_MAX_RANK], nval[MYHDF_MAX_RANK];
  int il,ib;
  void *buf = NULL;

  /* Check the parameters */
  if (this == (Output_t *)NULL) 
    RETURN_ERROR("invalid input structure", "PutOutputLine", false);
  if (!this->open)
    RETURN_ERROR("file not open", "PutOutputLine", false);

  for (ib = 0; ib < this->nband; ib++)
  {
      for (il = 0; il < this->size.l; il++)
      {
          /* Write the data */
          start[0] = il;
          start[1] = 0;
          nval[0] = 1;
          nval[1] = this->size.s;
          buf = (void *)final_mask[il];
          if (SDwritedata(this->sds[ib].id, start, NULL, nval, buf) == 
              HDF_ERROR)
              RETURN_ERROR("writing output", "PutOutputLine", false);
       }
  }
  
  return true;
}


bool PutMetadata(Output_t *this, Input_t *input)
{
  Myhdf_attr_t attr;
  char date[MAX_DATE_LEN + 1];
  double dval[NBAND_REFL_MAX];
  char process_ver[100];        /* CFmask processing version */
  int ib;

  /* Check the parameters */

  if (!this->open)
    RETURN_ERROR("file not open", "PutMetadata", false);

  /* Write the metadata */

  attr.id = -1;

  if (input->meta.provider != NULL) {
    attr.type = DFNT_CHAR8;
    attr.nval = strlen(input->meta.provider);
    attr.name = OUTPUT_PROVIDER;
    if (!PutAttrString(this->sds_file_id, &attr, input->meta.provider))
      RETURN_ERROR("writing attribute (data provider)", "PutMetadata", false);
  }

  if (input->meta.sat != NULL) {
    attr.type = DFNT_CHAR8;
    attr.nval = strlen(input->meta.sat);
    attr.name = OUTPUT_SAT;
    if (!PutAttrString(this->sds_file_id, &attr, input->meta.sat))
      RETURN_ERROR("writing attribute (satellite)", "PutMetadata", false);
  }

  if (input->meta.inst != NULL) {
    attr.type = DFNT_CHAR8;
    attr.nval = strlen(input->meta.inst);
    attr.name = OUTPUT_INST;
    if (!PutAttrString(this->sds_file_id, &attr, input->meta.inst))
      RETURN_ERROR("writing attribute (instrument)", "PutMetadata", false);
  }

  if (!FormatDate(&input->meta.acq_date, DATE_FORMAT_DATEA_TIME, date))
    RETURN_ERROR("formating acquisition date", "PutMetadata", false);

  attr.type = DFNT_CHAR8;
  attr.nval = strlen(date);
  attr.name = OUTPUT_ACQ_DATE;
  if (!PutAttrString(this->sds_file_id, &attr, date))
    RETURN_ERROR("writing attribute (acquisition date)", "PutMetadata", false);

  if (!FormatDate(&input->meta.prod_date, DATE_FORMAT_DATEA_TIME, date))
    RETURN_ERROR("formating production date", "PutMetadata", false);

  attr.type = DFNT_CHAR8;
  attr.nval = strlen(date);
  attr.name = OUTPUT_L1_PROD_DATE;
  if (!PutAttrString(this->sds_file_id, &attr, date))
    RETURN_ERROR("writing attribute (production date)", "PutMetadata", false);

  sprintf (process_ver, "%s", CFMASK_VERSION);
  attr.type = DFNT_CHAR8;
  attr.nval = strlen(process_ver);
  attr.name = OUTPUT_CFMASK_VERSION;
  if (!PutAttrString(this->sds_file_id, &attr, process_ver))
    RETURN_ERROR("writing attribute (CFmask Version)", "PutMetadata", false);

  attr.type = DFNT_FLOAT32;
  attr.nval = 1;
  attr.name = OUTPUT_SUN_ZEN;
  dval[0] = (double)input->meta.sun_zen;
  if (!PutAttrDouble(this->sds_file_id, &attr, dval))
    RETURN_ERROR("writing attribute (solar zenith)", "PutMetadata", false);

  attr.type = DFNT_FLOAT32;
  attr.nval = 1;
  attr.name = OUTPUT_SUN_AZ;
  dval[0] = (double)input->meta.sun_az;
  if (!PutAttrDouble(this->sds_file_id, &attr, dval))
    RETURN_ERROR("writing attribute (solar azimuth)", "PutMetadata", false);

  if (input->meta.wrs_sys != NULL) {
    attr.type = DFNT_CHAR8;
    attr.nval = strlen(input->meta.wrs_sys);
    attr.name = OUTPUT_WRS_SYS;
    if (!PutAttrString(this->sds_file_id, &attr, input->meta.wrs_sys))
      RETURN_ERROR("writing attribute (WRS system)", "PutMetadata", false);

    attr.type = DFNT_INT16;
    attr.nval = 1;
    attr.name = OUTPUT_WRS_PATH;
    dval[0] = (double)input->meta.path;
    if (!PutAttrDouble(this->sds_file_id, &attr, dval))
      RETURN_ERROR("writing attribute (WRS path)", "PutMetadata", false);

    attr.type = DFNT_INT16;
    attr.nval = 1;
    attr.name = OUTPUT_WRS_ROW;
    dval[0] = (double)input->meta.row;
    if (!PutAttrDouble(this->sds_file_id, &attr, dval))
      RETURN_ERROR("writing attribute (WRS row)", "PutMetadata", false);
  }

    /* output the upper left and lower right corners if they are available */
    if (!input->meta.ul_corner.is_fill)
    {
        attr.type = DFNT_FLOAT64;
        attr.nval = 2;
        attr.name = OUTPUT_UL_LAT_LONG;
        dval[0] = input->meta.ul_corner.lat;
        dval[1] = input->meta.ul_corner.lon;
        if (!PutAttrDouble(this->sds_file_id, &attr, dval))
            RETURN_ERROR("writing attribute (Upper-left lat/lon)", 
                         "PutMetadata", false);
    }

    if (!input->meta.lr_corner.is_fill)
    {
        attr.type = DFNT_FLOAT64;
        attr.nval = 2;
        attr.name = OUTPUT_LR_LAT_LONG;
        dval[0] = input->meta.lr_corner.lat;
        dval[1] = input->meta.lr_corner.lon;
        if (!PutAttrDouble(this->sds_file_id, &attr, dval))
            RETURN_ERROR("writing attribute (Lower-right lat/lon)", 
                         "PutMetadata", false);
    }

    /* output the geographic bounding coordinates if they are available */
    if (!input->meta.bounds.is_fill)
    {
        attr.type = DFNT_FLOAT64;
        attr.nval = 1;
        attr.name = OUTPUT_WEST_BOUND;
        dval[0] = input->meta.bounds.min_lon;
        if (!PutAttrDouble(this->sds_file_id, &attr, dval))
            RETURN_ERROR("writing attribute (West Bounding Coords)", 
                         "PutMetadata", false);

        attr.type = DFNT_FLOAT64;
        attr.nval = 1;
        attr.name = OUTPUT_EAST_BOUND;
        dval[0] = input->meta.bounds.max_lon;
        if (!PutAttrDouble(this->sds_file_id, &attr, dval))
            RETURN_ERROR("writing attribute (East Bounding Coords)", 
                         "PutMetadata", false);

        attr.type = DFNT_FLOAT64;
        attr.nval = 1;
        attr.name = OUTPUT_NORTH_BOUND;
        dval[0] = input->meta.bounds.max_lat;
        if (!PutAttrDouble(this->sds_file_id, &attr, dval))
            RETURN_ERROR("writing attribute (North Bounding Coords)", 
                         "PutMetadata", false);

        attr.type = DFNT_FLOAT64;
        attr.nval = 1;
        attr.name = OUTPUT_SOUTH_BOUND;
        dval[0] = input->meta.bounds.min_lat;
        if (!PutAttrDouble(this->sds_file_id, &attr, dval))
            RETURN_ERROR("writing attribute (South Bounding Coords)", 
                         "PutMetadata", false);
    }  /* if geographic bounds are not fill */

  /* now write out the fmask band attributes */
  for (ib = 0; ib < this->nband; ib++)
  {
      attr.type = DFNT_CHAR8;
      attr.nval = strlen(LONG_NAME);
      attr.name = OUTPUT_LONG_NAME;
      if (!PutAttrString(this->sds[ib].id, &attr, LONG_NAME))
         RETURN_ERROR("writing attribute (long name)", "PutMetadata", false);

      attr.type = DFNT_INT16;
      attr.nval = 2;
      attr.name = OUTPUT_VALID_RANGE;
      dval[0] = (double)(0);
      dval[1] = (double)(4);
      if (!PutAttrDouble(this->sds[ib].id, &attr, dval))
        RETURN_ERROR("writing attribute (valid range ref)","PutMetadata",false);

      attr.type = DFNT_INT16;
      attr.nval = 1;
      attr.name = OUTPUT_FILL_VALUE;
      dval[0] = (double)(255);
      if (!PutAttrDouble(this->sds[ib].id, &attr, dval))
        RETURN_ERROR("writing attribute (fill value)","PutMetadata",false);

      attr.type = DFNT_CHAR8;
      attr.nval = strlen(MASK_INDEX);
      attr.name = OUTPUT_MASK_INDEX;
      if (!PutAttrString(this->sds[ib].id, &attr, MASK_INDEX))
        RETURN_ERROR("writing attribute (mask index)", "PutMetadata", false);
  }

  return true;
}

