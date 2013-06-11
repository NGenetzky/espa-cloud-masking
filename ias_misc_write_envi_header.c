
/* Standard C Includes */
#include <stdio.h>
#include <limits.h>
#include <string.h>

/* IAS Includes */
#include "ias_types.h"
#include "ias_logging.h"
#include "error.h"
#include "input.h"

#ifndef ERROR
#define ERROR -1
#endif

#define SUCCESS 0

/*****************************************************************************
 NAME: ias_misc_write_envi_header

 PURPOSE: A simple routine to write an ENVI header for a specified image file.

 RETURN VALUE: Type = int
    Value    Description
    -------  ---------------------
    SUCCESS  Successful completion
    ERROR    Operation failed

 NOTES: The following are non-supported ENVI data types.
        6 = 2x32-bit complex, real-imaginary pair of double precision
        9 = 2x64-bit double precision complex, real-imaginary pair of double
                precision
        14 = 64-bit signed long integer
        15 = 64-bit unsigned long integer

*****************************************************************************/
int ias_misc_write_envi_header
(
    const char *image_filename, /* I: Full path name of the image file */
    const IAS_PROJECTION *proj_info, /* I: Optional projection info, set to 
                                           NULL if not known or needed */
    const char *description,    /* I: Optional description, set to NULL if not 
                                      known or needed */
    int lines,                  /* I: Number of lines in the data */
    int samples,                /* I: Number of samples in the data */
    int bands,                  /* I: Number of bands in the data */
    double upper_left_x,        /* I: Optional upper-left X coordinate, set to 
                                      0.0 if not known or needed (requires
                                      proj_info) */
    double upper_left_y,        /* I: Optional upper-left Y coordinate, set to 
                                      0.0 if not known or needed (requires
                                      proj_info) */
    double projection_distance_x, /* Optional pixel size in X projection, set
                                     to 0.0 if not known or needed (requires
                                     proj_info) */
    double projection_distance_y, /* Optional pixel size in Y projection, set 
                                     to 0.0 if not known or needed (requires
                                     proj_info) */
    IAS_DATA_TYPE data_type     /* I: The IAS type of the data */
)
{
    char output_filename[PATH_MAX]; /* ENVI header filename */
    FILE *output_fd; /* File descriptor for the output file */
    int envi_data_type; /* The IAS_DATA_TYPE converted to the ENVI equivalent */
    int count;          /* The number of characters printed */
    int status;

    /* Check to make sure image_filename is not NULL */
    if (image_filename == NULL)
    {
        IAS_LOG_ERROR("The image_filename is not correctly provided");
        return -1;
    }

    switch (data_type)
    {
        case IAS_BYTE:
            envi_data_type = 1;  /*  8-bit byte */
            break;
        case IAS_I2:
            envi_data_type = 2;  /* 16-bit signed integer */
            break;
        case IAS_I4:
            envi_data_type = 3;  /* 32-bit signed long integer */
            break;
        case IAS_R4:
            envi_data_type = 4;  /* 32-bit floating point */
            break;
        case IAS_R8:
            envi_data_type = 5;  /* 64-bit double precision floating point */
            break;
        case IAS_UI2:
            envi_data_type = 12; /* 16-bit unsigned integer */
            break;
        case IAS_UI4:
            envi_data_type = 13; /* 32-bit unsigned long integer */
            break;
        default:
            /* Output a warning message, because the caller may not care
               enough to halt processing */
            IAS_LOG_WARNING("Unsupported IAS_DATA_TYPE specified for ENVI"
                " header file creation %d", data_type);
            return -1;
    }

    /* Create output envi header filename */
    count = snprintf(output_filename, sizeof(output_filename),
            "%s.hdr", image_filename);
    if ((count < 0) || (count >= sizeof(output_filename)))
    {
        IAS_LOG_ERROR("Buffer for ENVI header filename requires %d "
                      "characters", count);
        return -1;
    }

    /* Create and write out the header file */
    output_fd = fopen(output_filename, "w");
    if (output_fd == NULL)
    {
        IAS_LOG_ERROR("Opening ENVI header file '%s' for writing",
            output_filename);
        return -1;
    }

    if (fprintf(output_fd, "ENVI\n") < 0)
    {
        IAS_LOG_ERROR("Writing ENVI header to: %s", output_filename);
        fclose(output_fd);
        return -1;
    }

    if (description != NULL)
    {
        status = fprintf(output_fd, "description = { %s }\n", 
                 description);
    }
    else
    {
        status = fprintf(output_fd, "description = { ENVI Header "
                 "for Image File %s}\n", image_filename);
    }
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing ENVI header description for "
                 "header_file: %s", output_filename);
        fclose(output_fd);
        return -1;
    }

    status = fprintf(output_fd,
            "lines = %d\n"
            "samples = %d\n"
            "bands = %d\n"
            "header offset = 0\n"
            "file type = ENVI Standard\n"
            "data type = %d\n"
            "interleave = bsq\n"
            "byte order = 0\n"
            "x start = 0\n"
            "y start = 0\n",
            lines, samples, bands, envi_data_type);
    if (status < 0)
    {
        IAS_LOG_ERROR("Writing ENVI header content for "
                      "header_file: %s",  output_filename);
        fclose(output_fd);
        return -1;
    }

    if (proj_info != NULL)
    {
        /* only supporting UTM for map projection information at 
           this time */
        if (proj_info->proj_code == 1)
        {
            char hemisphere[20];            
            char envi_proj[20];
            /* Always claim to be the northern hemisphere since we 
               aren't using the negative UTM zone code convention 
               for the southern hemisphere */
            strcpy(hemisphere, "North");
            strcpy(envi_proj, "UTM");

            if (fprintf(output_fd,"map info = {%s, 1.0, "
                "1.0, %f, %f, %f, %f, %d, %s, WGS-84, units=Meters}\n",
                envi_proj, upper_left_x, upper_left_y, 
                projection_distance_x, projection_distance_y, 
                proj_info->zone, hemisphere) < 0)
            {
                IAS_LOG_ERROR("Error writing ENVI header file %s", 
                              output_filename);
                fclose(output_fd);
                return -1;
            }
        }
        else
        {
            IAS_LOG_WARNING("Projection is not UTM, so not "
                "including map projection information in the ENVI "
                "header file");
        }
    }

    if (fclose(output_fd) != 0)
    {
        IAS_LOG_ERROR("Closing the ENVI header file: %s", 
                      output_filename);
        return -1;
    }

    return 0;
}

