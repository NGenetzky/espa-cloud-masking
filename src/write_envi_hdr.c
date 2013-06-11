#include "input.h"

/* ENVI projection numbers for UTM and PS */
#define ENVI_GEO_PROJ 1
#define ENVI_UTM_PROJ 2
#define ENVI_PS_PROJ 31

/* GCTP projection numbers for UTM and PS */
#define GCTP_GEO_PROJ 0
#define GCTP_UTM_PROJ 1
#define GCTP_PS_PROJ 6 

/******************************************************************************
MODULE:  write_envi_hdr

PURPOSE:  Writes the ENVI header to the specified file using the input info
provided.

RETURN VALUE:
Type = int
Value           Description
-----           -----------
-1              An error occurred generating the header file
0               Header file was successful

PROJECT:  Land Satellites Data System Science Research and Development (LSRD)
at the USGS EROS

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
2/8/2013    Gail Schmidt     Original Development
5/14/2013   Song Guo         Modified to support CFmask

NOTES:
  1. It's assumed the header file will be for unsigned byte products and
     therefore an ENVI data type of 1.
******************************************************************************/
int write_envi_hdr
(
    char *hdr_file,     /* I: name of header file to be generated */
    Input_t *toa_input, /* I: input structure for both the TOA reflectance
                              and brightness temperature products */
    Space_def_t *space_def /* I: spatial definition information */
)
{
    char FUNC_NAME[] = "write_envi_hdr";   /* function name */
    char errmsg[STR_SIZE];   /* error message */
    char *bin_file=NULL;     /* name of raw binary file */
    FILE *hdr_fptr=NULL;     /* file pointer to the ENVI header file */

    /* Open the header file */
    hdr_fptr = fopen (hdr_file, "w");
    if (hdr_fptr == NULL)
    {
        sprintf (errmsg, "Error opening %s for write access.", hdr_file);
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }

    /* Verify the projection is UTM or PS and sphere is WGS-84 */
    if (space_def->proj_num != GCTP_UTM_PROJ &&
        space_def->proj_num != GCTP_PS_PROJ)
    {
        sprintf (errmsg, "Error UTM projection code (%d) or PS projection "
            "code (%d) expected.", GCTP_UTM_PROJ, GCTP_PS_PROJ);
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }

    if (space_def->sphere != 12)
    {
        sprintf (errmsg, "Error WGS-84 sphere code (12) expected.");
        error_handler (true, FUNC_NAME, errmsg);
        return (-1);
    }

    /* Create the name of the associated raw binary file by replacing the
       .hdr of the header filename with .bin for the output binary file */
    bin_file = strdup (hdr_file);
    strncpy (&bin_file[strlen(hdr_file)-3], "bin", 3);

    /* Write the header to the file */
    fprintf (hdr_fptr,
        "ENVI\n"
        "description = {%s}\n"
        "samples = %d\n"
        "lines   = %d\n"
        "bands   = 1\n"
        "header offset = 0\n"
        "file type = ENVI Standard\n"
        "data type = 1\n"
        "interleave = bsq\n"
        "byte order = 0\n", bin_file, toa_input->size.s, toa_input->size.l);
   
    if (space_def->proj_num == GCTP_UTM_PROJ)
    {
        if (space_def->zone > 0)
            fprintf (hdr_fptr,
                "map info = {UTM, 1.000, 1.000, %f, %f, %f, %f, %d, North, "
                "WGS-84, units=Meters}\n", space_def->ul_corner.x,
                space_def->ul_corner.y, space_def->pixel_size,
                space_def->pixel_size, space_def->zone);
        else
            fprintf (hdr_fptr,
                "map info = {UTM, 1.000, 1.000, %f, %f, %f, %f, %d, South, "
                "WGS-84, units=Meters}\n", space_def->ul_corner.x,
                space_def->ul_corner.y, space_def->pixel_size,
                space_def->pixel_size, -(space_def->zone));
    }
    else if (space_def->proj_num == GCTP_PS_PROJ)
    {
        fprintf (hdr_fptr,
            "map info = {Polar Stereographic, 1.000, 1.000, %f, %f, %f, %f, "
            "WGS-84, units=Meters}\n", space_def->ul_corner.x,
            space_def->ul_corner.y, space_def->pixel_size,
            space_def->pixel_size);
        fprintf (hdr_fptr,
            "projection info = {%d, 6378137.0, 6356752.314245179, %lf, "
            "%lf, %lf, %lf, WGS-84, Polar Stereographic, units=Meters}\n",
            ENVI_PS_PROJ, space_def->proj_param[5], space_def->proj_param[4],
            space_def->proj_param[6], space_def->proj_param[7]);
    }

    fprintf (hdr_fptr, "band names = {Band 1}\n");

    /* Close the header file and free pointers */
    fclose (hdr_fptr);
    if (bin_file)
        free (bin_file);

    /* Successful completion */
    return (0);
}
