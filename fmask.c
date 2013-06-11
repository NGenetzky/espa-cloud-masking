#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#define CLOUD_PIXEL_BUFFER 3
#define CLOUD_SHADOW_PIXEL_BUFFER 3
#define CLOUD_PROBABILITY_THRESHOLD 22.5

int main (int argc, char * argv[])
{

    int cloud_pixel_buffer;
    int cloud_shadow_pixel_buffer;
    float cloud_probability_threshold;

    if ( argc < 2 )
    {
        sprintf (errstr, "Input filename was not provided");
        ERROR (errstr, "main");
    }

    if ( argc != 5 )
    {
        cloud_pixel_buffer = CLOUD_PIXEL_BUFFER;
        cloud_shadow_pixel_buffer = CLOUD_SHADOW_PIXEL_BUFFER;
        cloud_probability_threshold = CLOUD_PROBABILITY_THRESHOLD;
    }
    else
    {
        cloud_pixel_buffer = atoi(argv[2]);
        cloud_shadow_pixel_buffer = atoi(argv[3]);
        cloud_probability_threshold = atoi(argv[4]);
    }

    /* Get the input file name */
    input_file_name = strdup (argv[1]);
    if (input_file_name == NULL)
    {
        sprintf (errstr, "Input filename was not provided or memory "
            "allocation was not successful.");
        ERROR (errstr, "main");
    }
  

    /* Calls LEDAPS routines to read in needed metadata info and 
    band 1-5 and 7 TOA reflectance and band 6 BT */



    return 1;
}
