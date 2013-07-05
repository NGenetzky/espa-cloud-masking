#include <string.h>
#include <stdarg.h>
#include "input.h"
#include "output.h"
#include "cfmask.h"
#include "space.h"
#include "ias_logging.h"
#include "ias_misc_2d_array.h"

#define NUM_OUT_SDS 1
char *out_sds_names[NUM_OUT_SDS] = {"fmask_band"};
/******************************************************************************
MODULE:  cfmask

PURPOSE:  the main routine for fmask in C

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
3/15/2013   Song Guo         Original Development
5/14/2013   Song Guo         Added in Polar Stereographic support

NOTES: type ./cfmask --help for information to run the code
******************************************************************************/
int main (int argc, char *argv[])
{
    char errstr[MAX_STR_LEN];           /* error string */
    char lndcal_name[MAX_STR_LEN];
    char lndth_name[MAX_STR_LEN];
    char fmask_name[MAX_STR_LEN];
    char fmask_header[MAX_STR_LEN];
    char fmask_hdf_name[MAX_STR_LEN];
    char *lndmeta_name = NULL;          /* input lndmeta data filename */
    char directory[MAX_STR_LEN];
    char extension[MAX_STR_LEN];
    int ib;                             /* band counters */
    char sds_names[NBAND_REFL_MAX][MAX_STR_LEN]; /* array of image SDS names */
    Input_t *input = NULL;              /* input data and metadata */
    char  scene_name[MAX_STR_LEN];
    char description[MAX_STR_LEN];
    char *hdf_grid_name = "Grid";  /* name of the grid for HDF-EOS */
    unsigned char **cloud_mask;
    unsigned char **shadow_mask;
    unsigned char **snow_mask;
    unsigned char **water_mask;
    unsigned char **final_mask;
    int status;
    FILE *fd;
    float ptm;
    float t_templ;
    float t_temph;
    int out_sds_types[NUM_OUT_SDS];     /* array of image SDS types */
    char *sds_name="fmask_band";        /* Fmask hdf SDS name */
    Output_t *output = NULL;            /* output structure and metadata */
    bool verbose;            /* verbose flag for printing messages */
    bool write_binary;       /* should we write raw binary output? */
    bool no_hdf_output;      /* should we don't write HDF4 output file? */
    int cldpix;              /* Default buffer for cloud pixel dilate */
    int sdpix;               /* Default buffer for shadow pixel dilate */
    float cloud_prob;        /* Default cloud probability */
    Space_def_t space_def;   /* spatial definition information */
  
    /* Read the command-line arguments, including the name of the input
       Landsat TOA reflectance product and the DEM */
    status = get_args (argc, argv, &lndmeta_name, &cloud_prob, &cldpix,
                       &sdpix, &write_binary, &no_hdf_output, &verbose);
    if (status != 0)
    { 
        sprintf (errstr, "calling get_args");
        ERROR (errstr, "main");
    }

    ias_misc_split_filename(lndmeta_name, directory, scene_name, extension);
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
        printf("lndmeta_name=%s\n",lndmeta_name);
        printf("lndcal_name, lndth_name = %s, %s\n", lndcal_name, lndth_name); 
        printf("fmask_name, fmask_header, fmask_hdf_name = %s, %s, %s\n", 
                fmask_name, fmask_header, fmask_hdf_name); 
    }

    /* Open input file, read metadata, and set up buffers */
    input = OpenInput(lndth_name, lndcal_name, lndmeta_name);
    if (input == (Input_t *)NULL)
    {
        sprintf (errstr, "opening the input files: %s & %s", lndth_name,
                 lndcal_name);
        ERROR (errstr, "main");
    }

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
            printf ("DEBUG:   band stau_value_ref: %d\n", 
                   input->meta.satu_value_ref[ib]);
            printf ("DEBUG:   band stau_value_max: %d\n", 
                   input->meta.satu_value_max[ib]);
        }
        printf ("DEBUG: Thermal Band -->\n");
        printf ("DEBUG:   SDS name is %s\n", input->therm_sds.name);
        printf ("DEBUG:   SDS rank: %d\n", input->therm_sds.rank);
        printf ("DEBUG:   therm_stau_value_ref: %f\n", 
                   input->meta.therm_satu_value_ref);
        printf ("DEBUG:   therm_stau_value_max: %d\n", 
                   input->meta.therm_satu_value_max);

        printf("DEBUG: ROW is %d\n", input->meta.row);
        printf("DEBUG: PATH is %d\n", input->meta.path);
        printf("DEBUG: SUN AZIMUTH is %f\n", input->meta.sun_az);
        printf("DEBUG: SUN ZENITH is %f\n", input->meta.sun_zen);
        printf("DEBUG: Projection Zone is %d\n", input->meta.zone);

        printf("DEBUG: unit_ref is %s\n", input->meta.unit_ref);
        printf("DEBUG: valid_range_ref is %f & %f\n", 
           input->meta.valid_range_ref[0], 
           input->meta.valid_range_ref[1]);

        printf("DEBUG: UL projection X is %f\n", input->meta.ul_projection_x);
        printf("DEBUG: UL projection Y is %f\n", input->meta.ul_projection_y);
    }
    /* Copy the SDS names and QA SDS names from the input structure for the
       output structure, since we are simply duplicating the input */
    for (ib = 0; ib < input->nband; ib++)
        strcpy (&sds_names[ib][0], input->sds[ib].name);

    /* Dynamic allocate the 2d mask memory */
    cloud_mask = (unsigned char **)ias_misc_allocate_2d_array(input->size.l, 
                 input->size.s, sizeof(unsigned char)); 
    shadow_mask = (unsigned char **)ias_misc_allocate_2d_array(input->size.l, 
                 input->size.s, sizeof(unsigned char)); 
    snow_mask = (unsigned char **)ias_misc_allocate_2d_array(input->size.l, 
                 input->size.s, sizeof(unsigned char)); 
    water_mask = (unsigned char **)ias_misc_allocate_2d_array(input->size.l, 
                 input->size.s, sizeof(unsigned char)); 
    final_mask = (unsigned char **)ias_misc_allocate_2d_array(input->size.l, 
                 input->size.s, sizeof(unsigned char)); 
    if (cloud_mask == NULL  || shadow_mask == NULL || snow_mask == NULL
        || water_mask == NULL || final_mask == NULL)
    {
        sprintf (errstr, "Allocating mask memory");
        ERROR (errstr, "main");
    }

    /* Build the potential cloud, shadow, snow, water mask */
    status = potential_cloud_shadow_snow_mask(input, cloud_prob, &ptm,
             &t_templ, &t_temph, cloud_mask, shadow_mask, snow_mask, 
             water_mask, final_mask, verbose);
    if (status != SUCCESS)
    {
        sprintf (errstr, "calling potential_cloud_shadow_snow_mask");
        ERROR (errstr, "main");
    }

    printf("Pcloud done, starting cloud/shadow match\n");


    /* Build the final cloud shadow based on geometry matching and
       combine the final cloud, shadow, snow, water masks into fmask */
    status = object_cloud_shadow_match(input, ptm, t_templ, t_temph,
             cldpix, sdpix, cloud_mask, shadow_mask, snow_mask, water_mask,
             final_mask, verbose);
    if (status != SUCCESS)
    {
        sprintf (errstr, "calling object_cloud_and_shadow_match");
        ERROR (errstr, "main");
    }    

    if (verbose)
    {
        int cloud = 0;
        int shadow = 0;
        int water = 0;
        int snow = 0;
        int fill = 0;
        int row,col;
        int land_clear = 0;
        for (row = 0; row < input->size.l; row++)
        {
            for (col = 0; col < input->size.s; col++)
            {
                if (final_mask[row][col] == 1)
                    water++;
                if (final_mask[row][col] == 2)
                    shadow++;
                if (final_mask[row][col] == 3)
                    snow++;
                if (final_mask[row][col] == 4)
                    cloud++;
                if (final_mask[row][col] == 255)
                    fill++;
                if (final_mask[row][col] == 0)
                    land_clear++;
            }
        }
        printf("water, shadow, snow, cloud, land_clear, fill,  "
             "total_pixels=%d,%d, %d, %d, %d, %d, %d\n", water, shadow, snow,
             cloud, land_clear, fill, input->size.l*input->size.s);
    }
    status = ias_misc_free_2d_array((void **)shadow_mask);
    status = ias_misc_free_2d_array((void **)snow_mask);
    status = ias_misc_free_2d_array((void **)water_mask);
    status = ias_misc_free_2d_array((void **)cloud_mask);
    if (status != SUCCESS)
    {
        sprintf (errstr, "Freeing mask memory");
        ERROR (errstr, "main");
    }

    IAS_PROJECTION proj_info;
    proj_info.proj_code = 1;
    proj_info.zone = input->meta.zone;
    status = snprintf(description, sizeof(description), "Fmask for cloud, "
            "cloud shadow, snow, and water");
    if (status < 0 || status >= sizeof(description))
    {
        IAS_LOG_ERROR("Buffer for envi header description not large enough");
        exit(EXIT_FAILURE);
    }

    /* Get the projection and spatial information from the input TOA
       reflectance product */
    status = get_space_def_hdf (&space_def, lndcal_name, hdf_grid_name);
    if (status != SUCCESS)
    {
        IAS_LOG_ERROR("Reading spatial metadata from the HDF file: "
                      "%s", lndcal_name);
        exit(EXIT_FAILURE);
    }

    if (write_binary)
    {
        /* Create an ENVI header file for the binary fmask */
        status = write_envi_hdr(fmask_header, input, &space_def);
        if (status != SUCCESS)
        {
            IAS_LOG_ERROR("Creating ENVI header for binary fmask");
            exit(EXIT_FAILURE);
        }

        /* Open the mask file for writing */
        fd = fopen(fmask_name, "w"); 
        if (fd == NULL)
        {
            IAS_LOG_ERROR("Opening report file: %s", fmask_name);
            exit(EXIT_FAILURE);
        }

        /* Write out the mask file */
        status = fwrite(&final_mask[0][0], sizeof(unsigned char), input->size.l *
                 input->size.s, fd);
        if (status != input->size.l * input->size.s)
        {
            IAS_LOG_ERROR("Writing to %s", fmask_name);
            exit(EXIT_FAILURE);
        }

        /* Close the mask file */
        status = fclose(fd);
        if ( status )
        {
            IAS_LOG_ERROR("Closing file %s", fmask_name);
            exit(EXIT_FAILURE);
        }
    }

    if (!no_hdf_output)
    {
        /* Create and open fmask HDF output file */
        status = CreateOutput(fmask_hdf_name);
        if (status != true)
        {
            IAS_LOG_ERROR("Creating HDF fmask output file");
            exit(EXIT_FAILURE);
        }
        output = OpenOutput (fmask_hdf_name, sds_name, &input->size);
        if (output == NULL)
        {
            sprintf (errstr, "opening output file - %s", fmask_hdf_name);
            ERROR(errstr, "main");
        }

        if (!PutOutputLine(output, final_mask))
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
        CloseOutput (output);
        FreeOutput (output);

        /* Write the spatial information, after the file has been closed */
        out_sds_types[0] = DFNT_UINT8;
        if (put_space_def_hdf (&space_def, fmask_hdf_name, NUM_OUT_SDS, 
            out_sds_names, out_sds_types, hdf_grid_name) != 0)
        {
            IAS_LOG_ERROR("Putting spatial metadata to the HDF file: "
            "%s", lndcal_name);
            exit(EXIT_FAILURE);
        }
    }

    /* Close the input file and free the structure */
    CloseInput (input);
    FreeInput (input);

    /* Free the final_mask buffer */
    status = ias_misc_free_2d_array((void **)final_mask);
    if (status != SUCCESS)
    {
        sprintf (errstr, "Freeing cloud mask memory");
        ERROR (errstr, "main");
    }

    free(lndmeta_name);
    printf ("Processing complete.\n");
    return (SUCCESS);
}

/******************************************************************************
MODULE:  usage

PURPOSE:  Prints the usage information for this application.

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
3/15/2013   Song Guo         Original Development

NOTES: 
******************************************************************************/
void usage ()
{
    printf ("Fmask identify the cloud, shadow, snow, water and clear pixels using "
            "the input Landsat scene (top of atmosphere (TOA)reflection and "
            "brightness temperature (BT) for bamd 6) output from LEDAPS\n\n");
    printf ("usage: ./cfmask "
            "--metadata=input_metadata_filename_with_full_path "
            "--prob=input_cloud_probability_value "
            "--cldpix=input_cloud_pixel_buffer "
            "--sdpix=input_shadow_pixel_buffer "
            "[--write_binary] [--verbose]\n");

    printf ("\nwhere the following parameters are required:\n");
    printf ("    -metadata: name of the input metadata file (full path) output "
            "from LEDAPS\n");
    printf ("\nwhere the following parameters are optional:\n");
    printf ("    -prob: cloud_probability, default value is 22.5\n");
    printf ("    -cldpix: cloud_pixel_buffer for image dilate, default value is 3\n");
    printf ("    -sdpix: shadow_pixel_buffer for image dilate, default value is 3\n");
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
            "--metadata=/home/sguo/LEDAPS/ledaps-read-only/"
            "ledapsSrc/src/fmask2/L5010054_05420110312.metadata.txt "
            "--prob=22.5 "
            "--cldpix=3 "
            "--sdpix=3 "
            "--write_binary --verbose\n");
}
