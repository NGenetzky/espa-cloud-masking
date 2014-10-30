/*****************************************************************************
!File: input.c
*****************************************************************************/

#include "espa_metadata.h"
#include "espa_geoloc.h"
#include "raw_binary_io.h"

#include "const.h"
#include "error.h"
#include "cfmask.h"
#include "date.h"
#include "input.h"

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
bool
dn_to_bt (Input_t * input)
{
    float k1, k2;               /* constans */
    int dn = 255;               /* maximum DN value */
    float temp;                 /* intermediate variable */
    char errmsg[STR_SIZE];      /* error message */

    if (strcmp (input->meta.sat, "LANDSAT_7") == 0)
    {
        k1 = 666.09;
        k2 = 1282.71;
    }
    else if (strcmp (input->meta.sat, "LANDSAT_5") == 0)
    {
        k1 = 607.76;
        k2 = 1260.56;
    }
    else if (strcmp (input->meta.sat, "LANDSAT_4") == 0)
    {
        k1 = 671.62;
        k2 = 1284.30;
    }
    else
    {
        sprintf (errmsg, "Unsupported satellite sensor");
        RETURN_ERROR (errmsg, "dn_to_bt", false);
    }

    temp = (input->meta.gain_th * (float) dn) + input->meta.bias_th;
    temp = k2 / log ((k1 / temp) + 1.0);
    input->meta.therm_satu_value_max = (int) (100.0 * (temp - 273.15) + 0.5);

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
bool
dn_to_toa (Input_t * input)
{
    float esun[BI_REFL_BAND_COUNT]; /* earth sun distance for each band */
    int ib;                         /* band loop variable */
    int dn = 255;                   /* maximum DN value */
    float temp;                     /* intermediate variable */
    char errmsg[STR_SIZE];          /* error message */

    if (strcmp (input->meta.sat, "LANDSAT_7") == 0)
    {
        esun[BI_BLUE] = 1997.0;
        esun[BI_GREEN] = 1812.0;
        esun[BI_RED] = 1533.0;
        esun[BI_NIR] = 1039.0;
        esun[BI_SWIR_1] = 230.8;
        esun[BI_SWIR_2] = 84.9;
    }
    else if (strcmp (input->meta.sat, "LANDSAT_5") == 0)
    {
        esun[BI_BLUE] = 1983.0;
        esun[BI_GREEN] = 1796.0;
        esun[BI_RED] = 1536.0;
        esun[BI_NIR] = 1031.0;
        esun[BI_SWIR_1] = 220.0;
        esun[BI_SWIR_2] = 83.44;
    }
    else if (strcmp (input->meta.sat, "LANDSAT_4") == 0)
    {
        esun[BI_BLUE] = 1983.0;
        esun[BI_GREEN] = 1795.0;
        esun[BI_RED] = 1539.0;
        esun[BI_NIR] = 1028.0;
        esun[BI_SWIR_1] = 219.8;
        esun[BI_SWIR_2] = 83.49;
    }
    else
    {
        sprintf (errmsg, "Unsupported satellite sensor");
        RETURN_ERROR (errmsg, "dn_to_toa", false);
    }

    for (ib = 0; ib < BI_REFL_BAND_COUNT; ib++)
    {
        temp = (input->meta.gain[ib] * (float) dn) + input->meta.bias[ib];
        input->meta.satu_value_max[ib] = (int) ((10000.0 * PI * temp *
                                                 input->dsun_doy[input->meta.
                                                                 acq_date.
                                                                 doy -
                                                                 1] *
                                                 input->dsun_doy[input->meta.
                                                                 acq_date.
                                                                 doy -
                                                                 1]) /
                                                (esun[ib] *
                                                 cos (input->meta.sun_zen *
                                                      (PI / 180.0))) + 0.5);
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

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
2/13/2014   Gail Schmidt     Modified to work with ESPA internal raw binary
                             file format

!Design Notes:
******************************************************************************/
Input_t *OpenInput
(
    Espa_internal_meta_t *metadata /* I: input metadata */
)
{
    Input_t *this = NULL;
    char *error_string = NULL;
    int i;                      /* looping variable */
    int ib;                     /* band looping variable */
    int16 *buf = NULL;
    FILE *dsun_in = NULL;       /* EarthSunDistance.txt file pointer */
    char *path = NULL;
    char full_path[MAX_STR_LEN];
    int status;

    /* Create the Input data structure */
    this = (Input_t *) malloc (sizeof (Input_t));
    if (this == NULL)
        RETURN_ERROR ("allocating Input data structure", "OpenInput", NULL);

    /* Initialize and get input from header file */
    if (!GetXMLInput (this, metadata))
    {
        free (this);
        this = NULL;
        RETURN_ERROR ("getting input from header file", "OpenInput", NULL);
    }

    /* Open TOA reflectance files for access */
    for (ib = 0; ib < this->nband; ib++)
    {
        printf ("DEBUG: band %d filename: %s\n", ib, this->file_name[ib]);
        this->fp_bin[ib] = open_raw_binary (this->file_name[ib], "r");
        if (this->fp_bin[ib] == NULL)
        {
            RETURN_ERROR ("opening input TOA binary file", "OpenInput", NULL);
        }
        this->open[ib] = true;
    }

    /* Open thermal file for access */
    printf ("DEBUG: thermal band filename: %s\n", this->file_name_therm);
    this->fp_bin_therm = open_raw_binary (this->file_name_therm, "r");
    if (this->fp_bin_therm == NULL)
        error_string = "opening thermal binary file";
    else
        this->open_therm = true;

    /* Allocate input buffers.  Thermal band only has one band.  Image and QA
       buffers have multiple bands. */
    buf = calloc ((size_t) (this->size.s * this->nband), sizeof (int16));
    if (buf == NULL)
        error_string = "allocating input buffer";
    else
    {
        this->buf[0] = buf;
        for (ib = 1; ib < this->nband; ib++)
            this->buf[ib] = this->buf[ib - 1] + this->size.s;
    }

    this->therm_buf = calloc ((size_t) (this->size.s), sizeof (int16));
    if (this->therm_buf == NULL)
        error_string = "allocating input thermal buffer";

    if (error_string != NULL)
    {
        FreeInput (this);
        CloseInput (this);
        RETURN_ERROR (error_string, "OpenInput", NULL);
    }

    path = getenv ("ESUN");
    if (path == NULL)
    {
        error_string = "ESUN environment variable is not set";
        RETURN_ERROR (error_string, "OpenInput", NULL);
    }

    sprintf (full_path, "%s/%s", path, "EarthSunDistance.txt");
    dsun_in = fopen (full_path, "r");
    if (dsun_in == NULL)
    {
        error_string = "Can't open EarthSunDistance.txt file";
        RETURN_ERROR (error_string, "OpenInput", NULL);
    }

    for (i = 0; i < 366; i++)
    {
        if (fscanf (dsun_in, "%f", &this->dsun_doy[i]) == EOF)
        {
            error_string = "End of file (EOF) is met before 336 lines";
            RETURN_ERROR (error_string, "OpenInput", NULL);
        }
    }
    fclose (dsun_in);

    /* Calculate maximum TOA reflectance values and put them in metadata */
    status = dn_to_toa (this);
    if (!status)
    {
        error_string = "Error calling dn_to_toa routine";
        RETURN_ERROR (error_string, "OpenInput", NULL);
    }

    /* Calculate maximum BT values and put them in metadata */
    status = dn_to_bt (this);
    if (!status)
    {
        error_string = "Error calling dn_to_bt routine";
        RETURN_ERROR (error_string, "OpenInput", NULL);
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
bool
CloseInput (Input_t *this)
{
    int ib;
    bool none_open;

    if (this == NULL)
        RETURN_ERROR ("invalid input structure", "CloseInput", false);

    none_open = true;
    for (ib = 0; ib < this->nband; ib++)
    {
        if (this->open[ib])
        {
            none_open = false;
            close_raw_binary (this->fp_bin[ib]);
            this->open[ib] = false;
        }
    }

    if (this->open_therm)
    {
        close_raw_binary (this->fp_bin_therm);
        this->open_therm = false;
    }

    if (none_open)
        RETURN_ERROR ("no files open", "CloseInput", false);

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
bool
FreeInput (Input_t *this)
{
    int ib;

    if (this != (Input_t *) NULL)
    {
        for (ib = 0; ib < this->nband; ib++)
        {
            if (this->open[ib])
                RETURN_ERROR ("file still open", "FreeInput", false);
            free (this->file_name[ib]);
            this->file_name[ib] = NULL;
        }
        free (this->file_name_therm);
        this->file_name_therm = NULL;

        free (this);
        this = NULL;
    }

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
bool
GetInputLine (Input_t *this, int iband, int iline)
{
    void *buf = NULL;
    long loc; /* pointer location in the raw binary file */

    /* Check the parameters */
    if (this == (Input_t *) NULL)
        RETURN_ERROR ("invalid input structure", "GetIntputLine", false);
    if (iband < 0 || iband >= this->nband)
        RETURN_ERROR ("invalid band number", "GetInputLine", false);
    if (!this->open[iband])
        RETURN_ERROR ("file not open", "GetInputLine", false);
    if (iline < 0 || iline >= this->size.l)
        RETURN_ERROR ("invalid line number", "GetInputLine", false);

    /* Read the data */
    buf = (void *) this->buf[iband];
    loc = (long) (iline * this->size.s * sizeof (int16));
    if (fseek (this->fp_bin[iband], loc, SEEK_SET))
        RETURN_ERROR ("error seeking line (binary)", "GetInputLine", false);
    if (read_raw_binary
        (this->fp_bin[iband], 1, this->size.s, sizeof (int16),
         buf) != SUCCESS)
        RETURN_ERROR ("error reading line (binary)", "GetInputLine", false);

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
bool
GetInputThermLine (Input_t *this, int iline)
{
    void *buf = NULL;
    int i;            /* looping variable */
    long loc;         /* pointer location in the raw binary file */
    float therm_val;  /* tempoary thermal value for conversion from Kelvin to
                         Celsius */

    /* Check the parameters */
    if (this == (Input_t *) NULL)
        RETURN_ERROR ("invalid input structure", "GetIntputThermLine", false);
    if (!this->open_therm)
        RETURN_ERROR ("file not open", "GetInputThermLine", false);
    if (iline < 0 || iline >= this->size.l)
        RETURN_ERROR ("invalid line number", "GetInputThermLine", false);

    /* Read the data */
    buf = (void *) this->therm_buf;
    loc = (long) (iline * this->size.s * sizeof (int16));
    if (fseek (this->fp_bin_therm, loc, SEEK_SET))
        RETURN_ERROR ("error seeking thermal line (binary)",
                      "GetInputThermLine", false);
    if (read_raw_binary
        (this->fp_bin_therm, 1, this->size.s, sizeof (int16), buf) != SUCCESS)
        RETURN_ERROR ("error reading thermal line (binary)",
                      "GetInputThermLine", false);

    /* Convert from kelvin back to degrees Celsius since the application is
       based on Celsius.  If this is fill or saturated, then leave as fill or
       saturated. */
    for (i = 0; i < this->size.s; i++)
    {
        if (this->therm_buf[i] != this->meta.fill &&
            this->therm_buf[i] != this->meta.therm_satu_value_ref)
        {
            /* unscale and convert to celsius */
            therm_val = this->therm_buf[i] * this->meta.therm_scale_fact;
            therm_val -= 273.15;

            /* apply the old scale factor that the cfmask processing is based
               upon, with hard-coded values */
            therm_val *= 100.0;
            this->therm_buf[i] = (int) (therm_val + 0.5);
        }
    }

    return true;
}


#define DATE_STRING_LEN (50)
#define TIME_STRING_LEN (50)

/******************************************************************************
!Description: 'GetXMLInput' pulls input values from the XML structure.
 
!Input Parameters:
 this         'Input_t' data structure to be populated
 metadata     'Espa_internal_meta_t' data structure with XML info

!Output Parameters:
 (returns)      status:
                  'true' = okay (always returned)
                  'false' = error getting metadata from the XML file

!Team Unique Header:

! Design Notes:
  1. This replaces the previous GetInputMeta so the input values are pulled
     from the XML file instead of the HDF and MTL files.
******************************************************************************/
bool
GetXMLInput (Input_t *this, Espa_internal_meta_t *metadata)
{
    char *error_string = NULL;
    int ib;
    char acq_date[DATE_STRING_LEN + 1];
    char acq_time[TIME_STRING_LEN + 1];
    char temp[MAX_STR_LEN + 1];
    int i;                      /* looping variable */
    int indx = -1;              /* band index in XML file for band1 or band6 */
    Espa_global_meta_t *gmeta = &metadata->global; /* pointer to global meta */

    /* Initialize the input fields */
    this->nband = 0;
    for (ib = 0; ib < BI_REFL_BAND_COUNT; ib++)
    {
        this->file_name[ib] = NULL;
        this->open[ib] = false;
        this->fp_bin[ib] = NULL;
    }
    this->open_therm = false;
    this->file_name_therm = NULL;
    this->fp_bin_therm = NULL;

    /* Pull the appropriate data from the XML file */
    strcpy (acq_date, gmeta->acquisition_date);
    strcpy (acq_time, gmeta->scene_center_time);

    /* Make sure the acquisition time is not too long (i.e. contains too
       many decimal points for the date/time routines).  The time should be
       hh:mm:ss.ssssssZ (see DATE_FORMAT_DATEA_TIME in date.h) which is 16
       characters long.  If the time is longer than that, just chop it off. */
    if (strlen (acq_time) > 16)
        sprintf (&acq_time[15], "Z");

    strcpy (this->meta.sat, gmeta->satellite);
    this->meta.sun_zen = gmeta->solar_zenith;
    if (this->meta.sun_zen < -90.0 || this->meta.sun_zen > 90.0)
    {
        error_string = "solar zenith angle out of range";
        RETURN_ERROR (error_string, "GetXMLInput", true);
    }

    this->meta.sun_az = gmeta->solar_azimuth;
    if (this->meta.sun_az < -360.0 || this->meta.sun_az > 360.0)
    {
        error_string = "solar azimuth angle out of range";
        RETURN_ERROR (error_string, "GetXMLInput", true);
    }

    if (!strcmp (gmeta->instrument, "TM") ||
        !strncmp (gmeta->instrument, "ETM", 3))
    {
        /* reflectance bands */
        this->nband = BI_REFL_BAND_COUNT; /* number of reflectance bands */
    }
    else
    {
        error_string = "invalid instrument";
        RETURN_ERROR (error_string, "GetXMLInput", true);
    }

    /* Find L1G/T band 1 in the input XML file to obtain gain/bias
       information */
    for (i = 0; i < metadata->nbands; i++)
    {
        if (!strcmp (metadata->band[i].name, "band1")
            && !strncmp (metadata->band[i].product, "L1", 2))
        {
            this->meta.gain[BI_BLUE] = metadata->band[i].toa_gain;
            this->meta.bias[BI_BLUE] = metadata->band[i].toa_bias;
        }
        else if (!strcmp (metadata->band[i].name, "band2")
                 && !strncmp (metadata->band[i].product, "L1", 2))
        {
            this->meta.gain[BI_GREEN] = metadata->band[i].toa_gain;
            this->meta.bias[BI_GREEN] = metadata->band[i].toa_bias;
        }
        else if (!strcmp (metadata->band[i].name, "band3")
                 && !strncmp (metadata->band[i].product, "L1", 2))
        {
            this->meta.gain[BI_RED] = metadata->band[i].toa_gain;
            this->meta.bias[BI_RED] = metadata->band[i].toa_bias;
        }
        else if (!strcmp (metadata->band[i].name, "band4")
                 && !strncmp (metadata->band[i].product, "L1", 2))
        {
            this->meta.gain[BI_NIR] = metadata->band[i].toa_gain;
            this->meta.bias[BI_NIR] = metadata->band[i].toa_bias;
        }
        else if (!strcmp (metadata->band[i].name, "band5")
                 && !strncmp (metadata->band[i].product, "L1", 2))
        {
            this->meta.gain[BI_SWIR_1] = metadata->band[i].toa_gain;
            this->meta.bias[BI_SWIR_1] = metadata->band[i].toa_bias;
        }
        else if (!strcmp (metadata->band[i].name, "band7")
                 && !strncmp (metadata->band[i].product, "L1", 2))
        {
            this->meta.gain[BI_SWIR_2] = metadata->band[i].toa_gain;
            this->meta.bias[BI_SWIR_2] = metadata->band[i].toa_bias;
        }
        /* Thermal */
        else if (!strcmp (metadata->band[i].name, "band6")
                 && !strncmp (metadata->band[i].product, "L1", 2))
        {
            /* TM */
            this->meta.gain_th = metadata->band[i].toa_gain;
            this->meta.bias_th = metadata->band[i].toa_bias;
        }
        else if (!strcmp (metadata->band[i].name, "band61")
                 && !strncmp (metadata->band[i].product, "L1", 2))
        {
            /* ETM+ */
            this->meta.gain_th = metadata->band[i].toa_gain;
            this->meta.bias_th = metadata->band[i].toa_bias;
        }
    } /* for i */

    /* Find TOA band 1 in the input XML file to obtain band-related
       information */
    for (i = 0; i < metadata->nbands; i++)
    {
        if (!strcmp (metadata->band[i].name, "toa_band1") &&
            !strcmp (metadata->band[i].product, "toa_refl"))
        {
            /* this is the index we'll use for reflectance band info */
            indx = i;

            /* get the band1 info */
            this->file_name[BI_BLUE] = strdup (metadata->band[i].file_name);
            this->meta.satu_value_ref[BI_BLUE] =
                metadata->band[i].saturate_value;
        }
        else if (!strcmp (metadata->band[i].name, "toa_band2") &&
                 !strcmp (metadata->band[i].product, "toa_refl"))
        {
            this->file_name[BI_GREEN] = strdup (metadata->band[i].file_name);
            this->meta.satu_value_ref[BI_GREEN] =
                metadata->band[i].saturate_value;
        }
        else if (!strcmp (metadata->band[i].name, "toa_band3") &&
                 !strcmp (metadata->band[i].product, "toa_refl"))
        {
            this->file_name[BI_RED] = strdup (metadata->band[i].file_name);
            this->meta.satu_value_ref[BI_RED] =
                metadata->band[i].saturate_value;
        }
        else if (!strcmp (metadata->band[i].name, "toa_band4") &&
                 !strcmp (metadata->band[i].product, "toa_refl"))
        {
            this->file_name[BI_NIR] = strdup (metadata->band[i].file_name);
            this->meta.satu_value_ref[BI_NIR] =
                metadata->band[i].saturate_value;
        }
        else if (!strcmp (metadata->band[i].name, "toa_band5") &&
                 !strcmp (metadata->band[i].product, "toa_refl"))
        {
            this->file_name[BI_SWIR_1] = strdup (metadata->band[i].file_name);
            this->meta.satu_value_ref[BI_SWIR_1] =
                metadata->band[i].saturate_value;
        }
        else if (!strcmp (metadata->band[i].name, "toa_band7") &&
                 !strcmp (metadata->band[i].product, "toa_refl"))
        {
            this->file_name[BI_SWIR_2] = strdup (metadata->band[i].file_name);
            this->meta.satu_value_ref[BI_SWIR_2] =
                metadata->band[i].saturate_value;
        }
        /* Thermal */
        else if (!strcmp (metadata->band[i].name, "toa_band6") &&
                 !strcmp (metadata->band[i].product, "toa_bt"))
        {
            this->file_name_therm = strdup (metadata->band[i].file_name);
            this->meta.therm_satu_value_ref =
                metadata->band[i].saturate_value;
            this->meta.therm_scale_fact = metadata->band[i].scale_factor;
        }
    } /* for i */

    if (indx == -1)
    {
        error_string = "not able to find the reflectance index band";
        RETURN_ERROR (error_string, "GetXMLInput", true);
    }

    /* Pull the reflectance info from band1 in the XML file */
    this->size.s = metadata->band[indx].nsamps;
    this->size.l = metadata->band[indx].nlines;
    this->meta.fill = metadata->band[indx].fill_value;
    this->meta.pixel_size[0] = metadata->band[indx].pixel_size[0];
    this->meta.pixel_size[1] = metadata->band[indx].pixel_size[1];

    /* Convert the acquisition date/time values */
    sprintf (temp, "%sT%s", acq_date, acq_time);
    if (!DateInit (&this->meta.acq_date, temp, DATE_FORMAT_DATEA_TIME))
    {
        error_string = "converting acquisition date/time";
        RETURN_ERROR (error_string, "GetHeaderInput", false);
    }

    return true;
}
