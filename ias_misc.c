#include "input.h"
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <getopt.h>

#define MINSIGMA 1e-5
 
/******************************************************************************
MODULE:  

PURPOSE:  

RETURN: 

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
3/15/2013   Song Guo         Original Development

NOTES: 
******************************************************************************/
void prctile(int16 *array, int nums, int16 min, int16 max, float prct, 
                   float *result) 
{
    int *interval; 
    int i, j;

    int loops;

    loops = max - min +1;

    interval = calloc(loops, sizeof(int));

    for (i = 0; i < nums; i++)
    {
        interval[array[i]-min]++;
    }

    int sum = 0;
    for (j = 0; j < loops; j++)
    {
         sum += interval[j];
         if ((((float)sum/(float)nums) * 100 - prct) > MINSIGMA)
         {  
             *result = (float)(min + j);
             break;
         }
         else
         {
             continue;
         }
    }
    free(interval);
}

/******************************************************************************
MODULE:  

PURPOSE:  

RETURN: 

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
3/15/2013   Song Guo         Original Development

NOTES: 
******************************************************************************/
void prctile2(float *array, int nums, float min, float max, float prct, 
                   float *result) 
{
    int *interval; 
    int i, j;

    int start, end;
    int loops;

    start = (int)rint(min);
    end = (int)rint(max);

    loops = end - start + 1;

    interval = calloc(loops, sizeof(int));

    for (i = 0; i < nums; i++)
    {
        interval[(int) rint(array[i])-start]++;
    }

    int sum = 0;
    for (j = 0; j < loops; j++)
    {
         sum += interval[j];
         if ((((float)sum/(float)nums) * 100 - prct) > MINSIGMA)
         {  
             *result = (float)(start + j);
             break;
         }
         else
         {
             continue;
         }
    }
    free(interval);
}

/******************************************************************************
MODULE:  get_args

PURPOSE:  Gets the command-line arguments and validates that the required
arguments were specified.

RETURN VALUE:
Type = int
Value           Description
-----           -----------
-1              Error getting the command-line arguments or a command-line
                argument and associated value were not specified
0               No errors encountered

PROJECT:  Land Satellites Data System Science Research and Development (LSRD)
at the USGS EROS

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
1/2/2013    Gail Schmidt     Original Development
3/15/2013   Song Guo         Changed to support Fmask

NOTES:
  1. Memory is allocated for the input and output files.  All of these should
     be character pointers set to NULL on input.  The caller is responsible
     for freeing the allocated memory upon successful return.
******************************************************************************/
int get_args
(
    int argc,              /* I: number of cmd-line args */
    char *argv[],          /* I: string of cmd-line args */
    char **metadata_infile,/* O: address of input TOA filename */
    float *cloud_prob,     /* O: cloud_probability input */
    int *cldpix,           /* O: cloud_pixel buffer used for image dilate */
    int *sdpix,            /* O: shadow_pixel buffer used for image dilate  */
    bool *write_binary,    /* O: write raw binary flag */
    bool *verbose          /* O: verbose flag */
)
{
    int c;                         /* current argument index */
    int option_index;              /* index for the command-line option */
    static int verbose_flag=0;     /* verbose flag */
    static int binary_flag=0;      /* write binary flag */
    static int cldpix_default = 3; /* Default buffer for cloud pixel dilate */
    static int sdpix_default = 3;  /* Default buffer for shadow pixel dilate */
    static float cloud_prob_default = 22.5;   /* Default cloud probability */
    char errmsg[MAX_STR_LEN];      /* error message */
    char FUNC_NAME[] = "get_args"; /* function name */
    static struct option long_options[] =
    {
        {"verbose", no_argument, &verbose_flag, 1},
        {"write_binary", no_argument, &binary_flag, 1},
        {"metadata", required_argument, 0, 'm'},
        {"prob", required_argument, 0, 'p'},
        {"cldpix", required_argument, 0, 'c'},
        {"sdpix", required_argument, 0, 's'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    /* Assign the default values */
    *cloud_prob = cloud_prob_default;
    *cldpix = cldpix_default;
    *sdpix = sdpix_default;

    /* Loop through all the cmd-line options */
    opterr = 0;   /* turn off getopt_long error msgs as we'll print our own */
    while (1)
    {
        /* optstring in call to getopt_long is empty since we will only
           support the long options */
        c = getopt_long (argc, argv, "", long_options, &option_index);
        if (c == -1)
        {   /* Out of cmd-line options */
            break;
        }

        switch (c)
        {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;
     
            case 'h':  /* help */
                usage();
                return -1;
                break;

            case 'm':  /* toa infile */
                *metadata_infile = strdup (optarg);
                break;
     
            case 'p':  /* btemp infile */
                *cloud_prob = atof(optarg);
                break;
     
            case 'c':  /* dem infile */
                *cldpix = atoi(optarg);
                break;
     
            case 's':  /* snow cover outfile */
                *sdpix = atoi(optarg);
                break;
     
            case '?':
            default:
                sprintf (errmsg, "Unknown option %s", argv[optind-1]);
                error_handler (true, FUNC_NAME, errmsg);
                usage ();
                return -1;
                break;
        }
    }

    /* Make sure the infiles and outfiles were specified */
    if (*metadata_infile == NULL)
    {
        sprintf (errmsg, "Metadata input file is a required argument");
        error_handler (true, FUNC_NAME, errmsg);
        usage();
        return -1;
    }

    /* Check the write binary flag */
    if (binary_flag)
        *write_binary = true;
    else
        *write_binary = false;

    /* Check the verbose flag */
    if (verbose_flag)
        *verbose = true;
    else 
        *verbose = false;

    if (*verbose)
    {
        printf("metadata_input_file = %s\n", *metadata_infile);
        printf("cloud_probability = %f\n", *cloud_prob);
        printf("cloud_pixel_buffer = %d\n", *cldpix);
        printf("shadow_pixel_buffer = %d\n", *sdpix);
        printf("write_binary = %d\n", *write_binary);
        printf("verbose = %d\n", *verbose);
    }
    return 0;
}

/******************************************************************************
MODULE:  error_handler

PURPOSE:  Prints the error/warning message.

RETURN VALUE:
Type = None

PROJECT:  Land Satellites Data System Science Research and Development (LSRD)
at the USGS EROS

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
1/2/2012    Gail Schmidt     Original Development
3/15/2013   Song Guo         Used for Fmask

NOTES:
******************************************************************************/
void error_handler
(
    bool error_flag,  /* I: true for errors, false for warnings */
    char *module,     /* I: calling module name */
    char *errmsg      /* I: error message to be printed, without ending EOL */
)
{
    if (error_flag)
        printf ("Error: %s : %s\n\n", module, errmsg);
    else
        printf ("Warning: %s : %s\n", module, errmsg);
}
