
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "const.h"
#include "error.h"
#include "cfmask.h"

/******************************************************************************
MODULE:  prctile

PURPOSE: Calculate Percentile of an integer array 

RETURN: SUCCESS
        FAILURE

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
3/15/2013   Song Guo         Original Development

NOTES: 
******************************************************************************/
int prctile
(
    int16 * array, /*I: input data pointer */
    int nums,      /*I: number of input data array */
    int16 min,     /*I: minimum value in the input data array */
    int16 max,     /*I: maximum value in the input data array  */
    float prct,    /*I: percentage threshold */
    float *result  /*O: percentile calculated */
)
{
    int *interval;            /* array to store data in an interval */
    int i, j;                 /* loop variables */
    int loops;                /* data range for input data */
    char errstr[MAX_STR_LEN]; /* error string */

    /* Just return 0 if no input value */
    if (nums == 0)
        *result = 0.0;

    loops = max - min + 1;

    interval = calloc (loops, sizeof (int));
    if (interval == NULL)
    {
        sprintf (errstr, "Invalid memory allocation");
        RETURN_ERROR (errstr, "prctile", FAILURE);
    }

    for (i = 0; i < nums; i++)
    {
        interval[array[i] - min]++;
    }

    int sum = 0;
    for (j = 0; j < loops; j++)
    {
        sum += interval[j];
        if ((((float) sum / (float) nums) * 100 - prct) > MINSIGMA)
        {
            *result = (float) (min + j);
            break;
        }
        else
        {
            continue;
        }
    }
    free (interval);

    return SUCCESS;
}

/******************************************************************************
MODULE:  prctile2

PURPOSE:  Calculate Percentile of a floating point array

RETURN: SUCCESS
        FAILURE

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
3/15/2013   Song Guo         Original Development

NOTES: 
******************************************************************************/
int prctile2
(
    float *array, /*I: input data pointer */
    int nums,     /*I: number of input data array */
    float min,    /*I: minimum value in the input data array */
    float max,    /*I: maximum value in the input data array  */
    float prct,   /*I: percentage threshold */
    float *result /*O: percentile calculated */
)
{
    int *interval;            /* array to store data in an interval */
    int i, j;                 /* loop variables */
    int start, end;           /* start/end variables */
    int loops;                /* data range of input data */
    char errstr[MAX_STR_LEN]; /* error string */

    /* Just return 0 if no input value */
    if (nums == 0)
        *result = 0.0;

    start = (int) rint (min);
    end = (int) rint (max);

    loops = end - start + 2;

    interval = calloc (loops, sizeof (int));
    if (interval == NULL)
    {
        sprintf (errstr, "Invalid memory allocation");
        RETURN_ERROR (errstr, "prctile2", FAILURE);
    }

    for (i = 0; i < nums; i++)
    {
        interval[(int) rint (array[i]) - start]++;
    }

    int sum = 0;
    for (j = 0; j < loops; j++)
    {
        sum += interval[j];
        if ((((float) sum / (float) nums) * 100 - prct) > MINSIGMA)
        {
            *result = (float) (start + j);
            break;
        }
        else
        {
            continue;
        }
    }
    free (interval);

    return SUCCESS;
}

/******************************************************************************
MODULE:  get_args

PURPOSE:  Gets the command-line arguments and validates that the required
arguments were specified.

RETURN VALUE:
Type = int
Value           Description
-----           -----------
FAILURE         Error getting the command-line arguments or a command-line
                argument and associated value were not specified
SUCCESS         No errors encountered

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
1/2/2013    Gail Schmidt     Original Development
3/15/2013   Song Guo         Changed to support Fmask
9/13/2013   Song Guo         Changed to use RETURN_ERROR
2/19/2014   Gail Schmidt     Modified to utilize the ESPA internal raw binary
                             file format

NOTES:
  1. Memory is allocated for the input and output files.  All of these should
     be character pointers set to NULL on input.  The caller is responsible
     for freeing the allocated memory upon successful return.
******************************************************************************/
int get_args
(
    int argc,              /* I: number of cmd-line args */
    char *argv[],          /* I: string of cmd-line args */
    char **xml_infile,     /* O: address of input XML filename */
    float *cloud_prob,     /* O: cloud_probability input */
    int *cldpix,           /* O: cloud_pixel buffer used for image dilate */
    int *sdpix,            /* O: shadow_pixel buffer used for image dilate */
    int *max_cloud_pixels, /* O: Max cloud pixel number to divide cloud */
    bool * verbose         /* O: verbose flag */
)
{
    int c;                         /* current argument index */
    int option_index;              /* index for the command-line option */
    static int verbose_flag = 0;   /* verbose flag */
    static int cldpix_default = 3; /* Default buffer for cloud pixel dilate */
    static int sdpix_default = 3;  /* Default buffer for shadow pixel dilate */
    static int max_pixel_default = 0; /* Default maxium cloud pixel number for
                                         cloud division, 0 means no division */
    static float cloud_prob_default = 22.5; /* Default cloud probability */
    char errmsg[MAX_STR_LEN];               /* error message */
    char FUNC_NAME[] = "get_args";          /* function name */
    static struct option long_options[] = {
        {"verbose", no_argument, &verbose_flag, 1},
        {"xml", required_argument, 0, 'i'},
        {"prob", required_argument, 0, 'p'},
        {"cldpix", required_argument, 0, 'c'},
        {"sdpix", required_argument, 0, 's'},
        {"max_cloud_pixels", required_argument, 0, 'x'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    /* Assign the default values */
    *cloud_prob = cloud_prob_default;
    *cldpix = cldpix_default;
    *sdpix = sdpix_default;
    *max_cloud_pixels = max_pixel_default;

    /* Loop through all the cmd-line options */
    opterr = 0; /* turn off getopt_long error msgs as we'll print our own */
    while (1)
    {
        /* optstring in call to getopt_long is empty since we will only
           support the long options */
        c = getopt_long (argc, argv, "", long_options, &option_index);
        if (c == -1)
        {
            /* Out of cmd-line options */
            break;
        }

        switch (c)
        {
        case 0:
            /* If this option set a flag, do nothing else now. */
            if (long_options[option_index].flag != 0)
                break;

        case 'h':              /* help */
            usage ();
            return FAILURE;
            break;

        case 'i':              /* xml infile */
            *xml_infile = strdup (optarg);
            break;

        case 'p':              /* cloud probability value */
            *cloud_prob = atof (optarg);
            break;

        case 'c':              /* cloud pixel value for image dilation */
            *cldpix = atoi (optarg);
            break;

        case 's':              /* snow pixel value for image dilation */
            *sdpix = atoi (optarg);
            break;

        case 'x':              /* maxium cloud pixel number for cloud division,
                                   0 means no division */
            *max_cloud_pixels = atoi (optarg);
            break;

        case '?':
        default:
            sprintf (errmsg, "Unknown option %s", argv[optind - 1]);
            usage ();
            RETURN_ERROR (errmsg, FUNC_NAME, FAILURE);
            break;
        }
    }

    /* Make sure the infile was specified */
    if (*xml_infile == NULL)
    {
        sprintf (errmsg, "XML input file is a required argument");
        usage ();
        RETURN_ERROR (errmsg, FUNC_NAME, FAILURE);
    }

    /* Make sure this is some positive value */
    if (*max_cloud_pixels < 0)
    {
        sprintf (errmsg, "max_cloud_pixels must be >= 0");
        RETURN_ERROR (errmsg, FUNC_NAME, FAILURE);
    }

    /* Check the verbose flag */
    if (verbose_flag)
        *verbose = true;
    else
        *verbose = false;

    if (*verbose)
    {
        printf ("XML_input_file = %s\n", *xml_infile);
        printf ("cloud_probability = %f\n", *cloud_prob);
        printf ("cloud_pixel_buffer = %d\n", *cldpix);
        printf ("shadow_pixel_buffer = %d\n", *sdpix);
        printf ("max_cloud_pixels = %d\n", *max_cloud_pixels);
    }

    return SUCCESS;
}
