/*****************************************************************************
!File: output.c
  
!Description: Functions creating and writting data to the product output file.

!Revision History:
 Revision 1.0 2012/10/22
 Gail Schmidt
 Original Version - borrowed from some of the LEDAPS libraries.

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
#define OUTPUT_PGEVERSION ("PGEVersion")
#define OUTPUT_PROCESSVERSION ("ProcessVersion")

#define OUTPUT_WEST_BOUND  ("WestBoundingCoordinate")
#define OUTPUT_EAST_BOUND  ("EastBoundingCoordinate")
#define OUTPUT_NORTH_BOUND ("NorthBoundingCoordinate")
#define OUTPUT_SOUTH_BOUND ("SouthBoundingCoordinate")

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
  int ir;

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
  if (this->file_name == (char *)NULL) {
    free(this);
    RETURN_ERROR("duplicating file name", "OpenOutput", NULL);
  }

  this->open = false;
  this->size.l = size->l;
  this->size.s = size->s;
  this->sds.name = NULL;
  this->sds.dim[0].name = NULL;
  this->sds.dim[1].name = NULL;

  /* Open file for SD access */
  this->sds_file_id = SDstart((char *)file_name, DFACC_RDWR);
  if (this->sds_file_id == HDF_ERROR) {
    free(this->file_name);
    free(this);  
    RETURN_ERROR("opening output file for SD access", "OpenOutput", NULL); 
  }
  this->open = true;

  /* Set up the image SDSs */
  sds = &this->sds;
  sds->rank = 2;
  sds->type = DFNT_UINT8;
  sds->name = DupString(sds_names);
  if (sds->name == NULL) {
      error_string = "duplicating sds name";
      RETURN_ERROR(error_string, "CreateOutput", false); 
    }

  dim[0] = &sds->dim[0];
  dim[1] = &sds->dim[1];

  dim[0]->nval = this->size.l;
  dim[1]->nval = this->size.s;

  dim[0]->type = dim[1]->type = sds->type;

  dim[0]->name = DupString("YDim_Grid");
    if (dim[0]->name == NULL) {
      error_string = "duplicating dim name (l)";
      RETURN_ERROR(error_string, "CreateOutput", false); 
    }
  dim[1]->name = DupString("XDim_Grid");
    if (dim[1]->name == NULL) {
      error_string = "duplicating dim name (s)";
      RETURN_ERROR(error_string, "CreateOutput", false); 
    }

  if (!PutSDSInfo(this->sds_file_id, sds)) {
      error_string = "setting up the SDS";
      RETURN_ERROR(error_string, "CreateOutput", false); 
    }

  for (ir = 0; ir < sds->rank; ir++) {
      if (!PutSDSDimInfo(sds->id, dim[ir], ir)) {
        error_string = "setting up the dimension";
        RETURN_ERROR(error_string, "CreateOutput", false); 
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
  if (!this->open)
    RETURN_ERROR("file not open", "CloseOutput", false);

  /* Close image SDSs */
    if (SDendaccess(this->sds.id) == HDF_ERROR) 
      RETURN_ERROR("ending sds access", "CloseOutput", false);

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
  int ir;

  if (this->open) 
    RETURN_ERROR("file still open", "FreeOutput", false);

  if (this != NULL) {
    /* Free image band SDSs */
      for (ir = 0; ir < this->sds.rank; ir++) {
        if (this->sds.dim[ir].name != NULL) 
          free(this->sds.dim[ir].name);
      }
      if (this->sds.name != NULL) 
        free(this->sds.name);
    }

    if (this->file_name != NULL)
      free(this->file_name);
    free(this);

  return true;
}

/******************************************************************************
!Description: 'PutOutputLine' writes a line of data to the output file.
 
!Input Parameters:
 this           'output' data structure; the following fields are written:
                buf -- contains the line to be written
 iband          current band to be written (0-based)
 iline          current line to be written (0-based)

!Output Parameters:
 this           'output' data structure; the following fields are modified:
 (returns)      status:
                  'true' = okay
                  'false' = error return

!Team Unique Header:

!Design Notes:
*****************************************************************************/
bool PutOutputLine(Output_t *this, unsigned char **final_mask)
{
  int32 start[MYHDF_MAX_RANK], nval[MYHDF_MAX_RANK];
  int il;
  void *buf = NULL;

  /* Check the parameters */
  if (this == (Output_t *)NULL) 
    RETURN_ERROR("invalid input structure", "PutOutputLine", false);
  if (!this->open)
    RETURN_ERROR("file not open", "PutOutputLine", false);
#if 0
  int row,col;
  for (row = 0; row < this->size.l; row++)
   {
    for (col = 0; col <this->size.s; col++)
     {
  if (row == 0 && final_mask[row][col]!= 255)
   printf("row,col,mask=%d,%d,%d\n",row,col,final_mask[row][col]);
     }
   }
#endif 

  for (il = 0; il < this->size.l; il++)
  {
      /* Write the data */
      start[0] = il;
      start[1] = 0;
      nval[0] = 1;
      nval[1] = this->size.s;
      buf = (void *)final_mask[il];

      if (SDwritedata(this->sds.id, start, NULL, nval, buf) == HDF_ERROR)
        RETURN_ERROR("writing output", "PutOutputLine", false);
  }
  
      return true;
}

bool PutMetadata(Output_t *this, Input_t *input)
{
  Myhdf_attr_t attr;
  char date[MAX_DATE_LEN + 1];
  double dval[NBAND_REFL_MAX];
  char *message = {"mask_value = 0: clear; mask_value = 1: water; "
                        "mask_value = 2: shadow; mask_value = 3: shadow; "
                        "mask_value = 4: cloud; mask_value = 255: fill;"};

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

  attr.type = DFNT_FLOAT32;
  attr.nval = 1;
  attr.name = OUTPUT_SUN_ZEN;
  dval[0] = (double)input->meta.sun_zen * DEG;
  if (!PutAttrDouble(this->sds_file_id, &attr, dval))
    RETURN_ERROR("writing attribute (solar zenith)", "PutMetadata", false);

  attr.type = DFNT_FLOAT32;
  attr.nval = 1;
  attr.name = OUTPUT_SUN_AZ;
  dval[0] = (double)input->meta.sun_az * DEG;
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

#if 0
  attr.type = DFNT_FLOAT64;
  attr.nval = 1;
  attr.name = OUTPUT_WEST_BOUND;
  dval[0] = input->meta.bounds.min_lon * DEG;
  if (!PutAttrDouble(this->sds_file_id, &attr, dval))
    RETURN_ERROR("writing attribute (West Bounding Coords)", "PutMetadata", false);

  attr.type = DFNT_FLOAT64;
  attr.nval = 1;
  attr.name = OUTPUT_EAST_BOUND;
  dval[0] = input->meta.bounds.max_lon * DEG;
  if (!PutAttrDouble(this->sds_file_id, &attr, dval))
    RETURN_ERROR("writing attribute (East Bounding Coords)", "PutMetadata", false);

  attr.type = DFNT_FLOAT64;
  attr.nval = 1;
  attr.name = OUTPUT_NORTH_BOUND;
  dval[0] = input->meta.bounds.max_lat * DEG;
  if (!PutAttrDouble(this->sds_file_id, &attr, dval))
    RETURN_ERROR("writing attribute (North Bounding Coords)", "PutMetadata", false);

  attr.type = DFNT_FLOAT64;
  attr.nval = 1;
  attr.name = OUTPUT_SOUTH_BOUND;
  dval[0] = input->meta.bounds.min_lat * DEG;
  if (!PutAttrDouble(this->sds_file_id, &attr, dval))
    RETURN_ERROR("writing attribute (South Bounding Coords)", "PutMetadata", false);
#endif
    attr.type = DFNT_INT16;
    attr.nval = 1;
    attr.name = OUTPUT_FILL_VALUE;
    dval[0] = (double)255;
    if (!PutAttrDouble(this->sds_file_id, &attr, dval))
      RETURN_ERROR("writing attribute (valid range ref)","PutMetadata",false);

    attr.type = DFNT_CHAR8;
    attr.nval = strlen(message);
    attr.name = "Fmask index";
    if (!PutAttrString(this->sds_file_id, &attr, message))
      RETURN_ERROR("writing attribute (Fmask index)", "PutMetadata", false);

  return true;
}

