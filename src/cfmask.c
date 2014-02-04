#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "input.h"
#include "output.h"
#include "cfmask.h"
#include "space.h"
#include "2d_array.h"
#include "const.h"

#define NUM_OUT_SDS 1
#define PI (3.141592653589793238)
char *out_sds_names[NUM_OUT_SDS] = {"fmask_band"};
/******************************************************************************
METHOD:  cfmask

PURPOSE:  the main routine for fmask in C

RETURN VALUE:
Type = int
Value           Description
-----           -----------
ERROR           An error occurred during processing of the cfmask
SUCCESS         Processing was successful

PROJECT:  Land Satellites Data System Science Research and Development (LSRD)
at the USGS EROS

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
3/15/2013   Song Guo         Original Development
5/14/2013   Song Guo         Added in Polar Stereographic support
7/17/2013   Song Guo         Added in Map info 
8/15/2013   Song Guo         Modified to use TOA reflectance file 
                             as input instead of metadata file

NOTES: type ./cfmask --help for information to run the code
******************************************************************************/
int main (int argc, char *argv[])
{
    char errstr[MAX_STR_LEN];           /* error string */
    char lndth_name[MAX_STR_LEN];       /* ledaps Brightness Temperature file */
    char fmask_name[MAX_STR_LEN];       /* output fmask binary file name */
    char fmask_header[MAX_STR_LEN];     /* output fmask binary file header */
    char fmask_hdf_name[MAX_STR_LEN];   /* output fmask HDF file name */
    char fmask_hdf_hdr[MAX_STR_LEN];    /* output fmask HDF file header */
    char *lndcal_name = NULL;           /* input lndcal data filename */
    char directory[MAX_STR_LEN];        /* input/output data directory */
    char extension[MAX_STR_LEN];        /* input TOA file extension */
    int ib;                             /* band counters */
    char sds_names[NBAND_REFL_MAX][MAX_STR_LEN]; /* array of image SDS names */
    Input_t *input = NULL;              /* input data and meta data */
    char  scene_name[MAX_STR_LEN];      /* input data scene name */
    char *hdf_grid_name = "Grid";  /* name of the grid for HDF-EOS */
    unsigned char **cloud_mask;    /* cloud pixel mask */
    unsigned char **shadow_mask;   /* shadow pixel mask */
    unsigned char **snow_mask;     /* snow pixel mask */
    unsigned char **water_mask;    /* water pixel mask */
    int status;                    /* return value from function call */
    FILE *fd = NULL;               /* file pointer */
    float ptm;                     /* percent of clear-sky pixels */
    float t_templ = 0.0;     /* percentile of low background temperature */
    float t_temph = 0.0;     /* percentile of high background temperature */
    int out_sds_types[NUM_OUT_SDS];     /* array of image SDS types */
    char *sds_name="fmask_band";        /* Fmask hdf SDS name */
    Output_t *output = NULL;            /* output structure and metadata */
    bool verbose;            /* verbose flag for printing messages */
    bool write_binary;       /* should we write raw binary output? */
    bool no_hdf_output;      /* should we don't write HDF4 output file? */
    int cldpix = 2;          /* Default buffer for cloud pixel dilate */
    int sdpix = 2;           /* Default buffer for shadow pixel dilate */
    float cloud_prob;        /* Default cloud probability */
    Space_def_t space_def;   /* spatial definition information */
    float sun_azi_temp = 0.0;/* Keep the original sun azimuth angle */
  
    time_t now;
    time(&now);
    printf("CFmask start_time=%s\n",ctime(&now));

    /* Read the command-line arguments, including the name of the input
       Landsat TOA reflectance product and the DEM */
    status = get_args (argc, argv, &lndcal_name, &cloud_prob, &cldpix,
                       &sdpix, &write_binary, &no_hdf_output, &verbose);
    if (status != SUCCESS)
    { 
        sprintf (errstr, "calling get_args");
        ERROR (errstr, "main");
    }

    split_filename(lndcal_name, directory, scene_name, extension);
    if (verbose)
        printf("directory, scene_name, extension=%s,%s,%s\n", 
            directory, scene_name, extension);
    sprintf(lndcal_name, "%slndcal.%s.hdf", directory, scene_name);
    sprintf(lndth_name, "%slndth.%s.hdf", directory, scene_name);
    sprintf(fmask_name, "%sfmask.%s.img", directory, scene_name);
    sprintf(fmask_header, "%sfmask.%s.img.hdr", directory, scene_name);
    sprintf(fmask_hdf_name, "%sfmask.%s.hdf", directory, scene_name);
    if (verbose)
    {
        printf("lndcal_name, lndth_name = %s, %s\n", lndcal_name, lndth_name); 
        printf("fmask_name, fmask_header, fmask_hdf_name = %s, %s, %s\n", 
                fmask_name, fmask_header, fmask_hdf_name); 
    }

    /* Open input file, read metadata, and set up buffers */
    input = OpenInput(lndth_name, lndcal_name);
    if (input == NULL)
    {
        sprintf (errstr, "opening the input files: %s & %s", lndth_name,
                 lndcal_name);
        ERROR (errstr, "main");
    }

    /* Get the projection and spatial information from the input TOA
       reflectance product */
    status = get_space_def_hdf(&space_def, lndcal_name, hdf_grid_name);
    if (status != SUCCESS)
    {
        sprintf(errstr, "Reading spatial metadata from the HDF file: %s", 
               lndcal_name);
        ERROR (errstr, "main");
    }
    input->meta.zone = space_def.zone;

    if (verbose)
    {
        /* Print some info to show how the input metadata works */
        printf ("DEBUG: Number of input TOA bands: %d\n", input->nband);
        printf ("DEBUG: Number of input thermal bands: %d\n", 1);
        printf ("DEBUG: Number of input lines: %d\n", input->size.l);
        printf ("DEBUG: Number of input samples: %d\n", input->size.s);
        printf ("DEBUG: Number of input TOA lines: %d\n", input->toa_size.l);
        printf ("DEBUG: Number of input TOA samples: %d\n", input->toa_size.s);
        printf ("DEBUG: Provider is %s\n", input->meta.provider);
        printf ("DEBUG: Satellite is %s\n", input->meta.sat);
        printf ("DEBUG: Instrument is %s\n", input->meta.inst);
        printf ("DEBUG: ACQUISITION_DATE.DOY is %d\n", input->meta.acq_date.doy);
        printf ("DEBUG: WRS system is %s\n", input->meta.wrs_sys);
        printf ("DEBUG: Path is %d\n", input->meta.path);
        printf ("DEBUG: Row is %d\n", input->meta.row);
        printf ("DEBUG: Fill value is %d\n", input->meta.fill);
        for (ib = 0; ib < input->nband; ib++)
        {
            printf ("DEBUG: Band %d-->\n", ib);
            printf ("DEBUG:   SDS name is %s\n", input->sds[ib].name);
            printf ("DEBUG:   SDS rank: %d\n", input->sds[ib].rank);
            printf ("DEBUG:   band satu_value_ref: %d\n", 
                   input->meta.satu_value_ref[ib]);
            printf ("DEBUG:   band satu_value_max: %d\n", 
                   input->meta.satu_value_max[ib]);
            printf ("DEBUG:   band gains: %f, band biases: %f\n", 
                    input->meta.gain[ib], input->meta.bias[ib]);      
        }
        printf ("DEBUG: Thermal Band -->\n");
        printf ("DEBUG:   SDS name is %s\n", input->therm_sds.name);
        printf ("DEBUG:   SDS rank: %d\n", input->therm_sds.rank);
        printf ("DEBUG:   therm_satu_value_ref: %d\n", 
                   input->meta.therm_satu_value_ref);
        printf ("DEBUG:   therm_satu_value_max: %d\n", 
                   input->meta.therm_satu_value_max);
        printf ("DEBUG:   therm_gain: %f, therm_bias: %f\n", 
                input->meta.gain_th, input->meta.bias_th);

        printf("DEBUG: ROW is %d\n", input->meta.row);
        printf("DEBUG: PATH is %d\n", input->meta.path);
        printf("DEBUG: SUN AZIMUTH is %f\n", input->meta.sun_az);
        printf("DEBUG: SUN ZENITH is %f\n", input->meta.sun_zen);
        printf("DEBUG: Projection Zone is %d\n", input->meta.zone);

        printf("DEBUG: unit_ref is %s\n", input->meta.unit_ref);
        printf("DEBUG: valid_range_ref is %f & %f\n", 
           input->meta.valid_range_ref[0], 
           input->meta.valid_range_ref[1]);
    }

    /* If the scene is an ascending polar scene (flipped upside down), then
       the solar azimuth needs to be adjusted by 180 degrees.  The scene in
       this case would be north down and the solar azimuth is based on north
       being up clock-wise direction. Flip the south to be up will not change 
       the actual sun location, with the below relations, the solar azimuth
       angle will need add in 180.0 for correct sun location */
    if (input->meta.ul_corner.is_fill &&
        input->meta.lr_corner.is_fill &&
        (input->meta.ul_corner.lat - input->meta.lr_corner.lat) < MINSIGMA)
    {
        /* Keep the original solar azimuth angle */
        sun_azi_temp = input->meta.sun_az;
        input->meta.sun_az += 180.0;
        if ((input->meta.sun_az - 360.0) > MINSIGMA)
            input->meta.sun_az -= 360.0;
        if (verbose)
            printf ("  Polar or ascending scene.  Readjusting solar azimuth by "
                "180 degrees.\n  New value: %f degrees\n", input->meta.sun_az);
    }

    /* Copy the SDS names and QA SDS names from the input structure for the
       output structure, since we are simply duplicating the input */
    for (ib = 0; ib < input->nband; ib++)
        strcpy (&sds_names[ib][0], input->sds[ib].name);

    /* Dynamic allocate the 2d mask memory */
    cloud_mask = (unsigned char **)allocate_2d_array(input->size.l, 
                 input->size.s, sizeof(unsigned char)); 
    shadow_mask = (unsigned char **)allocate_2d_array(input->size.l, 
                 input->size.s, sizeof(unsigned char)); 
    snow_mask = (unsigned char **)allocate_2d_array(input->size.l, 
                 input->size.s, sizeof(unsigned char)); 
    water_mask = (unsigned char **)allocate_2d_array(input->size.l, 
                 input->size.s, sizeof(unsigned char)); 
    if (cloud_mask == NULL  || shadow_mask == NULL || snow_mask == NULL
        || water_mask == NULL)
    {
        sprintf (errstr, "Allocating mask memory");
        ERROR (errstr, "main");
    }

    /* Build the potential cloud, shadow, snow, water mask */
    status = potential_cloud_shadow_snow_mask(input, cloud_prob, &ptm,
             &t_templ, &t_temph, cloud_mask, shadow_mask, snow_mask, 
             water_mask, verbose);
    if (status != SUCCESS)
    {
        sprintf (errstr, "processing potential_cloud_shadow_snow_mask");
        ERROR (errstr, "main");
    }

    printf("Pcloud done, starting cloud/shadow match\n");


    /* Build the final cloud shadow based on geometry matching and
       combine the final cloud, shadow, snow, water masks into fmask */
    status = object_cloud_shadow_match(input, ptm, t_templ, t_temph,
             cldpix, sdpix, cloud_mask, shadow_mask, snow_mask, water_mask,
             verbose);
    if (status != SUCCESS)
    {
        sprintf (errstr, "processing object_cloud_and_shadow_match");
        ERROR (errstr, "main");
    }    

    status = free_2d_array((void **)shadow_mask);
    if (status != SUCCESS)
    {
        sprintf (errstr, "Freeing mask memory");
        ERROR (errstr, "main");
    }
    status = free_2d_array((void **)snow_mask);
    if (status != SUCCESS)
    {
        sprintf (errstr, "Freeing mask memory");
        ERROR (errstr, "main");
    }
    status = free_2d_array((void **)water_mask);
    if (status != SUCCESS)
    {
        sprintf (errstr, "Freeing mask memory");
        ERROR (errstr, "main");
    }

    /* Reassign solar azimuth angle for output purpose if south up north 
       down scene is involved */
    if (input->meta.ul_corner.is_fill &&
        input->meta.lr_corner.is_fill &&
        (input->meta.ul_corner.lat - input->meta.lr_corner.lat) < MINSIGMA)
        input->meta.sun_az = sun_azi_temp;
   
    if (write_binary)
    {
        /* Create an ENVI header file for the binary fmask */
        status = write_envi_hdr(fmask_header, BINARY_FILE, input, &space_def);
        if (status != SUCCESS)
        {
            sprintf(errstr, "Creating ENVI header for binary fmask");
            ERROR (errstr, "main");
        }

        /* Open the mask file for writing */
        fd = fopen(fmask_name, "w"); 
        if (fd == NULL)
        {
            sprintf(errstr, "Opening report file: %s", fmask_name);
            ERROR (errstr, "main");
        }

        /* Write out the mask file */
        status = fwrite(&cloud_mask[0][0], sizeof(unsigned char),
            input->size.l * input->size.s, fd);
        if (status != input->size.l * input->size.s)
        {
            sprintf(errstr, "Writing to %s", fmask_name);
            ERROR (errstr, "main");
        }

        /* Close the mask file */
        status = fclose(fd);
        if (status)
        {
            sprintf(errstr, "Closing file %s", fmask_name);
            ERROR (errstr, "main");
        }
    }

    if (!no_hdf_output)
    {
        /* Create and open fmask HDF output file */
        if (!CreateOutput(fmask_hdf_name))
        {
            sprintf(errstr, "Creating HDF fmask output file");
            ERROR (errstr, "main");
        }
        output = OpenOutput (fmask_hdf_name, sds_name, &input->size);
        if (output == NULL)
        {
            sprintf (errstr, "opening output file - %s", fmask_hdf_name);
            ERROR(errstr, "main");
        }

        if (!PutOutput(output, cloud_mask))
        {
            sprintf (errstr, "Writing output fmask in HDF files\n");
            ERROR (errstr, "main");
        }

        if (!PutMetadata(output, input))
        {
            sprintf (errstr, "Writing output fmask metadata in HDF files\n");
            ERROR (errstr, "main");
        }

        /* Close the output file and free the structure */
        if (!CloseOutput (output))
        {
            sprintf (errstr, "closing output file - %s", fmask_hdf_name);
            ERROR(errstr, "main");
        }
        if (!FreeOutput (output))
        {
            sprintf (errstr, "freeing output file - %s", fmask_hdf_name);
            ERROR(errstr, "main");
        }

        /* Write the spatial information, after the file has been closed */
        out_sds_types[0] = DFNT_UINT8;
        status = put_space_def_hdf (&space_def, fmask_hdf_name, NUM_OUT_SDS, 
            out_sds_names, out_sds_types, hdf_grid_name);
        if (status != SUCCESS)
        {
            sprintf("Putting spatial metadata to the HDF file: "
            "%s", lndcal_name);
            ERROR (errstr, "main");
        }

        /* Write CFmask HDF header to add in envi map info */
        sprintf (fmask_hdf_hdr, "%s.hdr", fmask_hdf_name);
        status = write_envi_hdr (fmask_hdf_hdr, HDF_FILE, input, &space_def);
        if (status != SUCCESS)
        {
            sprintf(errstr, "Error writing the ENVI header for CFmask HDF hdr");
            ERROR (errstr, "main");
        }
    }

    /* Free the final output cloud_mask */
    status = free_2d_array((void **)cloud_mask);
    if (status != SUCCESS)
    {
        sprintf (errstr, "Freeing mask memory");
        ERROR (errstr, "main");
    }

    /* Close the input file and free the structure */
    CloseInput (input);
    FreeInput (input);

    free(lndcal_name);
    printf ("Processing complete.\n");
    time(&now);
    printf("CFmask end_time=%s\n",ctime(&now));
    return (SUCCESS);
}

/******************************************************************************
MODULE:  usage

PURPOSE:  Prints the usage information for this application.

RETURN VALUE:
Type = None

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
3/15/2013   Song Guo         Original Development
8/15/2013   Song Guo         Modified to use TOA reflectance file 
                             as input instead of metadata file

NOTES: 
******************************************************************************/
void usage ()
{
    printf ("Fmask identify the cloud, shadow, snow, water and clear pixels using "
            "the input Landsat scene (top of atmosphere (TOA)reflection and "
            "brightness temperature (BT) for band 6) output from LEDAPS\n\n");
    printf ("usage: ./cfmask "
            "--toarefl=input_toarefl_filename_with_full_path "
            "--prob=input_cloud_probability_value "
            "--cldpix=input_cloud_pixel_buffer "
            "--sdpix=input_shadow_pixel_buffer "
            "[--write_binary] [--verbose]\n");

    printf ("\nwhere the following parameters are required:\n");
    printf ("    -toarefl: name of the input TOA reflectance file (full path) "
            "output from LEDAPS\n");
    printf ("\nwhere the following parameters are optional:\n");
    printf ("    -prob: cloud_probability, default value is 22.5\n");
    printf ("    -cldpix: cloud_pixel_buffer for image dilate, default value is 2\n");
    printf ("    -sdpix: shadow_pixel_buffer for image dilate, default value is 2\n");
    printf ("    -write_binary: should raw binary outputs and ENVI header "
            "files be written in addition to the HDF file? (default is false)"
            "\n");
    printf ("    -no_hdf_out: should Fmask not output as HDF file? "
            "(default is false)"
            "\n");
    printf ("    -verbose: should intermediate messages be printed? (default "
            "is false)\n");
    printf ("\n./fmask --help will print the usage statement\n");
    printf ("\nExample: ./cfmask "
            "--toarefl=/home/sguo/LEDAPS/ledaps-read-only/"
            "ledapsSrc/src/fmask2/lndcal.L5010054_05420110312.hdf "
            "--prob=22.5 "
            "--cldpix=3 "
            "--sdpix=3 "
            "--write_binary --verbose\n");
}
