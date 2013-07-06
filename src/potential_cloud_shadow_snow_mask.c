#include <string.h>
#include <stdarg.h>
#include <math.h>
#include "cv.h"
#include "ml.h"
#include "cxcore.h"
#include "highgui.h"
#include "ias_misc_2d_array.h"
#include "input.h"

#define MINSIGMA 1e-5

/******************************************************************************
MODULE: majority_filter 

PURPOSE: Simulate matlab majority function (The pixel is cloud pixel if 5 out of
         the 8 neighboring pixels plus the pixel itself are cloud pixels, 2 out
         of 4 if it's a corner pixel, 3 out of 6 if it's a boundary pixel)

RETURN: None

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
3/15/2013   Song Guo         Original Development

NOTES: 
******************************************************************************/
/* majority_filter will label a pixel as cloud_pixel when 5 of the 8
   neighboring pixels plus the pixel itself (totally 9) are labeled 
   as cloud pixels. If the pixel is at the corner, then if 2 out of 4 
   pixels are labeled as cloud pixel, then the corner pixel is labeled
   as cloud pixel. If the pixel is at the boundary line (not corner), 
   then if 3 out of 6 pixels are labeled as cloud pixel, then the 
   boundary line pixel is labeled as cloud pixel. */
void majority_filter(unsigned char **mask, int nrows, int ncols)
{
 int row, col;
 int cloud_pixels = 0;

   for (row = 0; row < nrows; row++)
   {
      for (col = 0; col < ncols; col++)
      {
          if ((row-1) > 0 && (row+1) < (nrows-1))
          {
              cloud_pixels = 0;
              if ((col-1) > 0 && (col+1) < (ncols-1))
              {
                  if (mask[row-1][col-1] == 1)
                      cloud_pixels++;
                  if (mask[row-1][col] == 1)
                      cloud_pixels++;
                  if (mask[row-1][col+1] == 1)
                      cloud_pixels++;
                  if (mask[row][col-1] == 1)
                      cloud_pixels++;
                  if (mask[row][col] == 1)
                      cloud_pixels++;
                  if (mask[row][col+1] == 1)
                      cloud_pixels++;
                  if (mask[row+1][col-1] == 1)
                      cloud_pixels++;
                  if (mask[row+1][col] == 1)
                      cloud_pixels++;
                  if (mask[row+1][col+1] == 1)
                      cloud_pixels++;
             }
         }
         if (cloud_pixels >= 5)
             mask[row][col] = 1;

         cloud_pixels = 0;
         if (row == 0 && col == 0)
         {
             if (mask[row][col] == 1)
                 cloud_pixels++;
             if (mask[row][col+1] == 1)
                 cloud_pixels++;
             if (mask[row+1][col] == 1)
                 cloud_pixels++;
             if (mask[row+1][col+1] == 1)
                 cloud_pixels++;
         }
         if (cloud_pixels >= 2)
             mask[row][col] = 1;

         cloud_pixels = 0;
         if (row == 0 && col == ncols-1)
         {
             if (mask[row][col-1] == 1)
                cloud_pixels++;
             if (mask[row][col] == 1)
                cloud_pixels++;
             if (mask[row+1][col-1] == 1)
                cloud_pixels++;
             if (mask[row+1][col] == 1)
                cloud_pixels++;
         }
         if (cloud_pixels >= 2)
             mask[row][col] = 1;

         cloud_pixels = 0;
         if (row == nrows-1 && col == 0)
         {
             if (mask[row-1][col] == 1)
                 cloud_pixels++;
             if (mask[row-1][col+1] == 1)
                 cloud_pixels++;
             if (mask[row][col] == 1)
                 cloud_pixels++;
             if (mask[row][col+1] == 1)
                 cloud_pixels++;
         }
         if (cloud_pixels >= 2)
             mask[row][col] = 1;

         cloud_pixels = 0;
         if (row == nrows-1 && col == ncols-1)
         {
             if (mask[row-1][col-1] == 1)
                 cloud_pixels++;
             if (mask[row-1][col] == 1)
                 cloud_pixels++;
             if (mask[row][col] == 1)
                 cloud_pixels++;
             if (mask[row][col-1] == 1)
                 cloud_pixels++;
         }
         if (cloud_pixels >= 2)
             mask[row][col] = 1;

         cloud_pixels = 0;
         if (row == 0 && (col > 0 && col < (ncols-1)))
         {
             if (mask[row][col-1] == 1)
                 cloud_pixels++;
             if (mask[row][col] == 1)
                 cloud_pixels++;
             if (mask[row][col+1] == 1)
                 cloud_pixels++;
             if (mask[row+1][col-1] == 1)
                 cloud_pixels++;
             if (mask[row+1][col] == 1)
                 cloud_pixels++;
             if (mask[row+1][col+1] == 1)
                 cloud_pixels++;
         }
         if (cloud_pixels >= 3)
             mask[row][col] = 1;

         cloud_pixels = 0;
         if (row == ncols-1 && (col > 0 && col < (ncols-1)))
         {
             if (mask[row][col-1] == 1)
                 cloud_pixels++;
             if (mask[row][col] == 1)
                 cloud_pixels++;
             if (mask[row][col+1] == 1)
                 cloud_pixels++;
             if (mask[row-1][col-1] == 1)
                 cloud_pixels++;
             if (mask[row-1][col] == 1)
                 cloud_pixels++;
             if (mask[row-1][col+1] == 1)
                 cloud_pixels++;
         }
         if (cloud_pixels >= 3)
             mask[row][col] = 1;

         cloud_pixels = 0;
         if ((row > 0 && row < (nrows-1)) && col == 0)
         {
             if (mask[row-1][col] == 1)
                 cloud_pixels++;
             if (mask[row][col] == 1)
                 cloud_pixels++;
             if (mask[row+1][col] == 1)
                 cloud_pixels++;
             if (mask[row-1][col+1] == 1)
                 cloud_pixels++;
             if (mask[row][col+1] == 1)
                 cloud_pixels++;
             if (mask[row+1][col+1] == 1)
                 cloud_pixels++;
         }
         if (cloud_pixels >= 3)
             mask[row][col] = 1;

         cloud_pixels = 0;
         if ((row > 0 && row < (nrows-1)) && col == ncols-1)
         {
             if (mask[row-1][col-1] == 1)
                 cloud_pixels++;
             if (mask[row][col-1] == 1)
                 cloud_pixels++;
             if (mask[row+1][col-1] == 1)
                 cloud_pixels++;
             if (mask[row-1][col] == 1)
                 cloud_pixels++;
             if (mask[row][col] == 1)
                 cloud_pixels++;
             if (mask[row+1][col-1] == 1)
                 cloud_pixels++;
         }
         if (cloud_pixels >= 3)
             mask[row][col] = 1;
      }
   }
}

/******************************************************************************
MODULE:  potential_cloud_shadow_snow_mask

PURPOSE: Identify the cloud pixels, snow pixels, water pixels, clear land 
         pixels, and potential shadow pixels

RETURN: SUCCESS
        ERROR

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
3/15/2013   Song Guo         Original Development

NOTES: 
******************************************************************************/
bool potential_cloud_shadow_snow_mask
(
    Input_t *input,
    float cloud_prob_threshold,
    float *ptm,
    float *t_templ,
    float *t_temph,
    unsigned char **cloud_mask,
    unsigned char **shadow_mask,
    unsigned char **snow_mask,
    unsigned char **water_mask,
    unsigned char **final_mask,
    bool verbose       
)
{
    char errstr[MAX_STR_LEN];
    int nrows = input->size.l;
    int ncols = input->size.s;
    int ib = 0;
    int row =0; 
    int col = 0;
    int mask_counter = 0;
    int clear_pixel_counter = 0;
    int clear_land_pixel_counter = 0;
    float ndvi, ndsi;
    int16 *f_temp = NULL;
    int16 *f_wtemp = NULL;
    float visi_mean;
    float whiteness = 0.0;
    float hot;
    float lndptm;
    float l_pt;
    float h_pt;
    float t_wtemp;
    float **wfinal_prob;
    float **final_prob;
    float wtemp_prob;
    int t_bright;
    float brightness_prob;
    int t_buffer;
    float temp_l;
    float temp_prob;
    float vari_prob;
    float max_value;
    float *prob = NULL;
    float clr_mask;
    float wclr_mask;
    int16 *nir = NULL;
    int16 *swir = NULL;
    float backg_b4;
    float backg_b5;
    int16 shadow_prob;
    int status;
    int satu_bv;

    /* Dynamic memory allocation */
    unsigned char **mask;
    unsigned char **clear_mask;
    unsigned char **clear_land_mask;

    mask = (unsigned char **)ias_misc_allocate_2d_array(input->size.l, 
                 input->size.s, sizeof(unsigned char)); 
    clear_mask = (unsigned char **)ias_misc_allocate_2d_array(input->size.l, 
                 input->size.s, sizeof(unsigned char)); 
    clear_land_mask = (unsigned char **)ias_misc_allocate_2d_array(
                 input->size.l, input->size.s, sizeof(unsigned char)); 
    if (mask == NULL || clear_mask == NULL || clear_land_mask ==NULL)
    {
        sprintf (errstr, "Allocating mask memory");
        ERROR (errstr, "pcloud");
    }
    
    if (verbose)
        printf("The first pass\n");
    /* Loop through each line in the image */
    for (row = 0; row < nrows; row++)
    {
        /* Print status on every 100 lines */
        if (!(row%1000)) 
        {
           if (verbose)
           {
               printf ("Processing line %d\r",row);
               fflush (stdout);
           }
        }

        /* For each of the image bands */
        for (ib = 0; ib < input->nband; ib++)
        {
            /* Read each input reflective band -- data is read into
               input->buf[ib] */
            if (!GetInputLine(input, ib, row))
            {
                sprintf (errstr, "Reading input image data for line %d, "
                    "band %d", row, ib);
                ERROR (errstr, "pcloud");
            }
        }

	/* For the thermal band */
	/* Read the input thermal band -- data is read into input->therm_buf */
	if (!GetInputThermLine(input, row))
        {
	    sprintf (errstr, "Reading input thermal data for line %d", row);
	    ERROR (errstr, "pcloud");
	}

        for (col = 0; col < ncols; col++)
        {
            int ib;
            for (ib = 0; ib <  NBAND_REFL_MAX; ib++)
            {
                 if (input->buf[ib][col] == 20000)
                 input->buf[ib][col] = input->meta.satu_value_max[ib]; 
            }
            if (input->therm_buf[col] == 20000)
                input->therm_buf[col] = input->meta.therm_satu_value_max; 

            /* process non-fill pixels only */
            if (input->therm_buf[col] <= -9999 || input->buf[0][col] == -9999 
                || input->buf[1][col] == -9999 || input->buf[2][col] == -9999 
                || input->buf[3][col] == -9999 || input->buf[4][col] == -9999
                || input->buf[5][col] == -9999)
                mask[row][col] = 0;
            else
            {
                 mask[row][col] = 1;
	         mask_counter++;
            }
  
            if ((input->buf[2][col] +  input->buf[3][col]) != 0 
                && mask[row][col] == 1)
                ndvi = (float)(input->buf[3][col] -  input->buf[2][col]) / 
                    (float)(input->buf[3][col] +  input->buf[2][col]);
            else
                ndvi = 0.01;

            if ((input->buf[1][col] +  input->buf[4][col]) != 0 
                && mask[row][col] == 1)
                ndsi = (float)(input->buf[1][col] -  input->buf[4][col]) / 
                (float)(input->buf[1][col] +  input->buf[4][col]);
            else
                ndsi = 0.01;

            /* Basic cloud test */
            if (((ndsi - 0.8) < MINSIGMA) && ((ndvi - 0.8) < MINSIGMA) && 
                (input->buf[5][col] > 300) && (input->therm_buf[col] < 2700))
                 cloud_mask[row][col] = 1;
            else
                 cloud_mask[row][col] = 0;

           /* It takes every snow pixels including snow pixel under thin 
              clouds or icy clouds */
           if (((ndsi - 0.15) > MINSIGMA) && (input->therm_buf[col] < 380) && 
               (input->buf[3][col] > 1100) && (input->buf[1][col] > 1000))
               snow_mask[row][col] = 1;
           else 
               snow_mask[row][col] = 0;

           /* Zhe's water test (works over thin cloud) */
          if (((((ndvi - 0.01) < MINSIGMA) && (input->buf[3][col] < 1100)) || 
              (((ndvi - 0.1) < MINSIGMA) && (ndvi > MINSIGMA) && 
              (input->buf[3][col] < 500))) && (mask[row][col]==1))
              water_mask[row][col] = 1;
          else 
              water_mask[row][col] = 0;
          if (mask[row][col] == 0)
              water_mask[row][col] = 255;

          /* visible bands flatness (sum(abs)/mean < 0.6 => brigt and dark 
             cloud) */
          if (cloud_mask[row][col] == 1 && mask[row][col] == 1)
          {
             visi_mean = (float)(input->buf[0][col] + input->buf[1][col] +
                  input->buf[2][col]) / 3.0;
             if (visi_mean != 0)
                 whiteness = ((fabs((float)input->buf[0][col] - visi_mean) + 
                     fabs((float)input->buf[1][col] - visi_mean) +
                     fabs((float)input->buf[2][col] - visi_mean)))/ visi_mean;
             else
                 whiteness = 100.0; /* Just put a large value to remove them 
                                       from cloud pixel identification */
          }         

          /* Update cloud_mask,  if one visible band is saturated, 
             whiteness = 0, due to data type conversion, pixel value difference
             of 1 is possible */
          if ((input->buf[0][col] >= (input->meta.satu_value_max[0]-1)) ||
	     (input->buf[1][col] >= (input->meta.satu_value_max[1]-1)) ||
	     (input->buf[2][col] >= (input->meta.satu_value_max[2]-1)))
          {
              whiteness = 0.0;
              satu_bv = 1;
          }
          else
          {
             satu_bv = 0;
          }

          if (cloud_mask[row][col] == 1 && (whiteness - 0.7) < MINSIGMA)
              cloud_mask[row][col] = 1; 
          else
              cloud_mask[row][col] = 0;

          /* Haze test */
          hot = (float)input->buf[0][col] - 0.5 * (float)input->buf[2][col] 
                 - 800.0;
          if (cloud_mask[row][col] == 1 && (hot > MINSIGMA || satu_bv == 1))
              cloud_mask[row][col] = 1;
          else
              cloud_mask[row][col] = 0;

          /* Ratio 4/5 > 0.75 test */
          if (cloud_mask[row][col] == 1 && input->buf[4][col] != 0)
          {
              if ((float)input->buf[3][col]/(float)(input->buf[4][col]) - 0.75 
                  > MINSIGMA)
                  cloud_mask[row][col] = 1;
              else
                  cloud_mask[row][col] = 0;
          }
          else
              cloud_mask[row][col] = 0;
     
          /* Test whether use thermal band or not */
          if (cloud_mask[row][col] == 0 && mask[row][col] == 1)
          {
	      clear_mask[row][col] = 1;
	      clear_pixel_counter++;
          }
          else
              clear_mask[row][col] = 0;

          if (water_mask[row][col] == 0 && clear_mask[row][col] == 1)
          {
	      clear_land_mask[row][col] = 1;
              clear_land_pixel_counter++;
          }
          else
          clear_land_mask[row][col] = 0;
       }
    }

    *ptm = 100. * ((float)clear_pixel_counter / (float)mask_counter);
    lndptm = 100. * ((float)clear_land_pixel_counter / (float)mask_counter);
    if (verbose)
        printf("*ptm, lndptm=%f,%f\n", *ptm, lndptm);

    if ((*ptm-0.1) <= MINSIGMA) /* No thermal test is needed, all clouds */
    {
        majority_filter(cloud_mask, nrows, ncols);
        *t_templ = -1.0;
        *t_temph = -1.0; 
        for (row = 0; row < nrows; row++)
        {
	    for (col = 0; col <ncols; col++)
	    {	    
	        /* All cloud */
	        if (cloud_mask[row][col] != 1)
	            shadow_mask[row][col] = 1;
                else
		    shadow_mask[row][col] = 0;
            }
        }
    }
    else
    {
        f_temp = malloc(input->size.l * input->size.s * sizeof(int16));
	f_wtemp = malloc(input->size.l * input->size.s * sizeof(int16));
	if (f_temp == NULL   || f_wtemp == NULL)
	{
	     sprintf (errstr, "Allocating temp memory");
	     ERROR (errstr, "pcloud");
	}
    
        if (verbose)
            printf("The second pass\n");
        int16 f_temp_max = 0;
        int16 f_temp_min = 0;
        int16 f_wtemp_max = 0;
        int16 f_wtemp_min = 0;
        int index = 0;
        int index2 = 0;
        /* Loop through each line in the image */
        for (row = 0; row < nrows; row++)
        {
            /* Print status on every 100 lines */
            if (!(row%1000)) 
            {
                if (verbose)
                {
	            printf ("Processing line %d\r", row);
                    fflush (stdout);
                }
	    }
	 
            /* For the thermal band, read the input thermal band  */
	    if (!GetInputThermLine(input, row))
	    {
	        sprintf (errstr,"Reading input thermal data for line %d", row);
	        ERROR (errstr, "pcloud");
	     }

             for (col =0; col < ncols; col++)
	     {
                 if (input->buf[5][col] == 20000)
                     input->buf[5][col] = input->meta.satu_value_max[5]; 
                 if (input->therm_buf[col] == 20000)
                     input->therm_buf[col] = input->meta.therm_satu_value_max; 

                 if ((lndptm-0.1) >= MINSIGMA)
	         {
                     /* get clear land temperature */
	             if (clear_land_mask[row][col] == 1)
	             {
		         f_temp[index] = input->therm_buf[col];
                         if (f_temp_max < f_temp[index])
                             f_temp_max = f_temp[index];
                         if (f_temp_min > f_temp[index])
                             f_temp_min = f_temp[index];
        	         index++;
	             }
                 }
        	 else
	         {
	             /*get clear water temperature */
	             if (clear_mask[row][col] == 1)
	             {
                         f_temp[index] = input->therm_buf[col];
                         if (f_temp_max < f_temp[index])
                             f_temp_max = f_temp[index];
                         if (f_temp_min > f_temp[index])
                             f_temp_min = f_temp[index];
	                 index++;
	             }
	         }
        	 if (water_mask[row][col] == 1 && input->buf[5][col] <= 300)
	         {
	             f_wtemp[index2] = input->therm_buf[col];
                     if (f_wtemp[index2] > f_wtemp_max)
                         f_wtemp_max = f_wtemp[index2];
                     if (f_wtemp[index2] < f_wtemp_min)
                         f_wtemp_min = f_wtemp[index2];
                     index2++;
	         }
             }
        }
    
         /* Tempearture for snow test */
         l_pt = 0.175;
         h_pt = 1.0 - l_pt;
         /* 0.175 percentile background temperature (low) */
         status = prctile(f_temp, index, f_temp_min, f_temp_max, 100.0*l_pt, 
                          t_templ);
         if (status != SUCCESS)
         {
	     sprintf (errstr, "Error calling prctile routine");
	     ERROR (errstr, "pcloud");
         }
         /* 0.825 percentile background temperature (high) */
         status = prctile(f_temp, index, f_temp_min, f_temp_max, 100.0*h_pt, 
                          t_temph);
         if (status != SUCCESS)
         {
	     sprintf (errstr, "Error calling prctile routine");
	     ERROR (errstr, "pcloud");
         }
         status = prctile(f_wtemp, index2, f_wtemp_min, f_wtemp_max, 
                          100.0*h_pt, &t_wtemp);
         if (status != SUCCESS)
         {
	     sprintf (errstr, "Error calling prctile routine");
	     ERROR (errstr, "pcloud");
         }

         /* Temperature test */
         t_buffer = 4*100;    
         *t_templ -= (float)t_buffer;
         *t_temph += (float)t_buffer;
         temp_l=*t_temph-*t_templ;

         /* Relase f_temp memory */
         free(f_wtemp);
         free(f_temp);

         wfinal_prob = (float **)ias_misc_allocate_2d_array(input->size.l, 
                 input->size.s, sizeof(float)); 
         final_prob = (float **)ias_misc_allocate_2d_array(input->size.l, 
                 input->size.s, sizeof(float)); 
         if (wfinal_prob == NULL   ||  final_prob == NULL)
         {
	     sprintf (errstr, "Allocating prob memory");
	     ERROR (errstr, "pcloud");
         }

         if (verbose)
             printf("The third pass\n");
         /* Loop through each line in the image */
         for (row = 0; row < nrows; row++)
        {
            /* Print status on every 100 lines */
            if (!(row%1000)) 
            {
                if (verbose)
                {
                    printf ("Processing line %d\r",row);
                    fflush (stdout);
                }
            }

            /* For each of the image bands */
            for (ib = 0; ib < input->nband; ib++)
            {
                /* Read each input reflective band -- data is read into
                   input->buf[ib] */
                if (!GetInputLine(input, ib, row))
                {
                    sprintf (errstr, "Reading input image data for line %d, "
                        "band %d", row, ib);
                    ERROR (errstr, "pcloud");
                }
            }

	    /* For the thermal band, data is read into input->therm_buf */
	    if (!GetInputThermLine(input, row))
            {
       	        sprintf (errstr, "Reading input thermal data for line %d", row);
	        ERROR (errstr, "pcloud");
	    }

            /* Loop through each line in the image */
	    for (col = 0; col <ncols; col++)
	    {
                for (ib = 0; ib <  NBAND_REFL_MAX - 1; ib++)
                {
                    if (input->buf[ib][col] == 20000)
                       input->buf[ib][col] = input->meta.satu_value_max[ib]; 
                }
                if (input->therm_buf[col] == 20000)
                   input->therm_buf[col] = input->meta.therm_satu_value_max; 

                if (water_mask[row][col] == 1)
                {
	            /* Get cloud prob over water */
	            /* Temperature test over water */
                    wtemp_prob = (t_wtemp - (float)input->therm_buf[col]) / 
                                  400.0;
	    
                    /* Brightness test (over water) */
	            t_bright = 1100;
	            brightness_prob = (float)input->buf[4][col] / 
                                      (float)t_bright;
	            if ((brightness_prob - 1.0) > MINSIGMA)
	                 brightness_prob = 1.0;
	    
                    /*Final prob mask (water), cloud over water probability */
	            wfinal_prob[row][col] =100.0 * wtemp_prob * brightness_prob;
                }
                else
                {	    
                    temp_prob=(*t_temph-(float)input->therm_buf[col]) / temp_l;
	            /* Temperature can have prob > 1 */
	            if (temp_prob < MINSIGMA)
	                temp_prob = 0.0;
	    
                    if ((input->buf[2][col] +  input->buf[3][col]) != 0 
                         && mask[row][col] == 1)
                        ndvi = (float)(input->buf[3][col] -  
                             input->buf[2][col]) / 
                             (float)(input->buf[3][col] +  input->buf[2][col]);
                    else
                        ndvi = 0.01;

                    if ((input->buf[1][col] +  input->buf[4][col]) != 0 
                        && mask[row][col] == 1)
                        ndsi = (float)(input->buf[1][col] -  
                            input->buf[4][col]) / 
                            (float)(input->buf[1][col] +  input->buf[4][col]);
                    else
                        ndsi = 0.01;

                    /* NDVI and NDSI should not be negative */
	            if (ndsi < MINSIGMA)
	                ndsi = 0.0;
	            if (ndvi < MINSIGMA)
	                ndvi = 0.0;
	    
                    visi_mean = (input->buf[0][col] + input->buf[1][col] +
                        input->buf[2][col]) / 3.0;
                    if (visi_mean != 0)
                        whiteness = (float)((fabs((float)input->buf[0][col] - 
                           visi_mean) + fabs((float)input->buf[1][col] - 
                           visi_mean) + fabs((float)input->buf[2][col] - 
                           visi_mean))) / visi_mean;
                    else
                        whiteness = 0.0;

                    /* If one visible band is saturated, whiteness = 0 */
                    if ((input->buf[0][col] >= (input->meta.satu_value_max[0]
                       - 1)) || 
                       (input->buf[1][col] >= (input->meta.satu_value_max[1]-1))
                       || (input->buf[2][col] >= (input->meta.satu_value_max[2]
                       -1)))
                           whiteness = 0.0;

                    /* Vari_prob=1-max(max(abs(NDSI),abs(NDVI)),whiteness); */
	            if ((ndsi - ndvi) > MINSIGMA) 
	                max_value = ndsi;
	            else
	                max_value = ndvi;
	            if ((whiteness - max_value) > MINSIGMA) 
	                max_value = whiteness;
	            vari_prob = 1.0 - max_value;

                    /*Final prob mask (land) */
	            final_prob[row][col] = 100.0 * (temp_prob * vari_prob);
                }
            }
        }

        prob = malloc(input->size.l * input->size.s * sizeof(float));
        if(prob == NULL)
        {
            sprintf (errstr, "Allocating prob memory");
	    ERROR (errstr, "pcloud");
        }

        float prob_max = 0.0;
        float prob_min = 0.0;
        int index3 = 0;
        for (row = 0; row < nrows; row++)
        {
	    for (col = 0; col <ncols; col++)
	    {	    
	        if (clear_land_mask[row][col] == 1)
	        {
		    prob[index3] = final_prob[row][col];
                    if ((prob[index3] - prob_max) > MINSIGMA)
                        prob_max = prob[index3];
                    if ((prob_min - prob[index3]) > MINSIGMA)
                        prob_min = prob[index3];
		    index3++;
	        }
            }
        }

        /*Dynamic threshold for land */
        status = prctile2(prob, index3, prob_min, prob_max, 100.0*h_pt, 
                          &clr_mask);
        clr_mask += cloud_prob_threshold;
        if (status != SUCCESS)
        {
	    sprintf (errstr, "Error calling prctile2 routine");
	    ERROR (errstr, "pcloud");
        }

        /* Relase memory for prob */
        free(prob);
	     
        /* Fixed threshold for water */
        wclr_mask = 50.0;
	    
        if (verbose)
            printf("pcloud probability threshold (land) = %.2f\n", clr_mask);
    
        /* Band 4 &5 flood fill */
        nir = malloc(input->size.l * input->size.s * sizeof(int16)); 
        swir = malloc(input->size.l * input->size.s * sizeof(int16)); 
        if (nir == NULL   || swir == NULL)
        {
            sprintf(errstr, "Allocating nir and swir memory");
            ERROR (errstr, "pcloud");
        }
        int16 nir_max = 0;
        int16 nir_min = 0;
        int16 swir_max = 0;
        int16 swir_min = 0;
        index = 0;
        index2 = 0;

        /* May need allocate two memory for new band 4 and 5 */
        int16 **new_nir;
        int16 **new_swir;   

        new_nir = (int16 **)ias_misc_allocate_2d_array(input->size.l, 
              input->size.s, sizeof(int16)); 
        new_swir = (int16 **)ias_misc_allocate_2d_array(input->size.l, 
              input->size.s, sizeof(int16)); 
        if (new_nir == NULL  || new_swir == NULL)
        {
            sprintf (errstr, "Allocating new_nir/new_swir memory");
            ERROR (errstr, "pcloud");
        }

        if (verbose)
            printf("The fourth pass\n");
        /* Loop through each line in the image */
        for (row = 0; row < nrows; row++)
        {
            /* Print status on every 100 lines */
            if (!(row%1000)) 
            {
                if (verbose)
                {
                    printf ("Processing line %d\r",row);
                    fflush (stdout);
                }
            }

            /* For each of the image bands */
            for (ib = 0; ib < input->nband; ib++)
            {
                /* Read each input reflective band -- data is read into
                   input->buf[ib] */
                if (!GetInputLine(input, ib, row))
                {
                    sprintf (errstr, "Reading input image data for line %d, "
                        "band %d", row, ib);
                    ERROR (errstr, "pcloud");
                }
            }

            /* For the thermal band, data is read into input->therm_buf */
	    if (!GetInputThermLine(input, row))
            {
	        sprintf (errstr, "Reading input thermal data for line %d", row);
	        ERROR (errstr, "pcloud");
	    }

            for (col =0; col < ncols; col++)
            {
                if (input->buf[3][col] == 20000)
                   input->buf[3][col] = input->meta.satu_value_max[3]; 
                if (input->buf[4][col] == 20000)
                   input->buf[4][col] = input->meta.satu_value_max[4]; 
                if (input->therm_buf[col] == 20000)
                   input->therm_buf[col] = input->meta.therm_satu_value_max; 

                if ((cloud_mask[row][col] == 1 && final_prob[row][col] > 
                     clr_mask && water_mask[row][col] == 0) || 
                     (cloud_mask[row][col] == 1 && wfinal_prob[row][col] > 
                     wclr_mask && water_mask[row][col] == 1) 
                     || (final_prob[row][col] > 99 && water_mask[row][col] == 
                     0) || (input->therm_buf[col] < *t_templ + t_buffer - 3500))
                    cloud_mask[row][col] = 1;
                else
                    cloud_mask[row][col] = 0;

                if (clear_land_mask[row][col] == 1)
	        {
                    nir[index] = input->buf[3][col];
                    if (nir[index] > nir_max)
                        nir_max = nir[index];
                    if (nir[index] < nir_min)
                        nir_min = nir[index];
	            index++;
	        }

                if (clear_land_mask[row][col] == 1)
	        {
                    swir[index2] = input->buf[4][col];
                    if (swir[index2] > swir_max)
                        swir_max = swir[index2];
                    if (swir[index2] < swir_min)
                        swir_min = swir[index2];
	            index2++;
	        }
                new_nir[row][col] = input->buf[3][col];
                new_swir[row][col] = input->buf[4][col];
            }
        }

        /* Free the memory */
        status = ias_misc_free_2d_array((void **)wfinal_prob);
        status = ias_misc_free_2d_array((void **)final_prob);
      
        /* Improve cloud mask by majority filtering */
        majority_filter(cloud_mask, nrows, ncols);

        /* Estimating background (land) Band 4 Ref */
        prctile(nir, index + 1, nir_min, nir_max, 100.0*l_pt, &backg_b4);
        prctile(swir, index2 + 1, swir_min, swir_max, 100.0*l_pt, &backg_b5);

        /* Release the memory */
        free(nir);
        free(swir);

        /* Open the intermediate file for writing */
        FILE *fd;
        fd = fopen("b4.bin", "wb"); 
        if (fd == NULL)
        {
            sprintf (errstr, "Opening file: b4.bin\n");
            ERROR (errstr, "pcloud");
        }

        /* Write out the intermediate file */
        status = fwrite(&new_nir[0][0], sizeof(int16), input->size.l *
                 input->size.s, fd);
        if (status != input->size.l * input->size.s)
        {
            sprintf (errstr, "Writing file: b4.bin\n");
            ERROR (errstr, "pcloud");
        }

        /* Close the intermediate file */
        status = fclose(fd);
        if ( status )
        {
            sprintf (errstr, "Closing file: b4.bin\n");
            ERROR (errstr, "pcloud");
        }
        fd = fopen("b5.bin", "wb"); 
        if (fd == NULL)
        {
            sprintf (errstr, "Opening file: b5.bin\n");
            ERROR (errstr, "pcloud");
        }

        /* Write out the intermediate file */
        status = fwrite(&new_swir[0][0], sizeof(int16), input->size.l *
                 input->size.s, fd);
        if (status != input->size.l * input->size.s)
        {
            sprintf (errstr, "Writing file: b5.bin\n");
            ERROR (errstr, "pcloud");
        }

        /* Close the intermediate file */
        status = fclose(fd);
        if ( status )
        {
            sprintf (errstr, "Closing file: b5.bin\n");
            ERROR (errstr, "pcloud");
        }
        fd = fopen("b4_b5.txt", "w"); 
        if (fd == NULL)
        {
            sprintf (errstr, "Opening file: b4_b5.txt\n");
            ERROR (errstr, "pcloud");
        }

        /* Write out the intermediate file */
        fprintf(fd, "%f\n", backg_b4);
        fprintf(fd, "%f\n", backg_b5);
        fprintf(fd, "%d\n", input->size.l);
        fprintf(fd, "%d\n", input->size.s);

        /* Close the intermediate file */
        status = fclose(fd);
        if ( status )
        {
            sprintf (errstr, "Closing file: b4_b5.txt\n");
            ERROR (errstr, "pcloud");
        }

        system("run_fillminima.py");

        /* Open the intermediate file for reading */
        fd = fopen("filled_b4.bin", "rb"); 
        if (fd == NULL)
        {
            sprintf (errstr, "Opening file: filled_b4.bin\n");
            ERROR (errstr, "pcloud");
        }

        /* Read out the intermediate file */
        fread(&new_nir[0][0], sizeof(int16), input->size.l *
                 input->size.s, fd);

        /* Close the intermediate file */
        status = fclose(fd);
        if ( status )
        {
            sprintf (errstr, "Closing file: filled_b4.bin\n");
            ERROR (errstr, "pcloud");
        }
        fd = fopen("filled_b5.bin", "rb"); 
        if (fd == NULL)
        {
            sprintf (errstr, "Opening file: filled_b5.bin\n");
            ERROR (errstr, "pcloud");
        }

        /* Read out the intermediate file */
        fread(&new_swir[0][0], sizeof(int16), input->size.l *
                 input->size.s, fd);

        /* Close the intermediate file */
        status = fclose(fd);
        if ( status )
        {
            sprintf (errstr, "Closing file: filled_b5.bin\n");
            ERROR (errstr, "pcloud");
        }

        system("rm b4_b5.txt");
        system("rm b4.bin");
        system("rm b5.bin");
        system("rm filled_b4.bin");
        system("rm filled_b5.bin");

        if (verbose)
            printf("The fifth pass\n");
        for (row = 0; row < nrows; row++)
        {
            /* Print status on every 100 lines */
            if (!(row%1000)) 
            {
                if (verbose)
                {
                    printf ("Processing line %d\r",row);
                    fflush (stdout);
                }
            }

            /* For each of the image bands */
            for (ib = 0; ib < input->nband; ib++)
            {
                /* Read each input reflective band -- data is read into
                   input->buf[ib] */
                if (!GetInputLine(input, ib, row))
                {
                    sprintf (errstr, "Reading input image data for line %d, "
                        "band %d", row, ib);
                    ERROR (errstr, "pcloud");
                }
            }

            for (col = 0; col < ncols; col++)
            {
                if (input->buf[3][col] == 20000)
                    input->buf[3][col] = input->meta.satu_value_max[3]; 
                if (input->buf[4][col] == 20000)
                    input->buf[4][col] = input->meta.satu_value_max[4]; 

                if (mask[row][col] == 1)
                { 
                     new_nir[row][col] -= input->buf[3][col];
                     new_swir[row][col] -= input->buf[4][col];

                     if (new_nir[row][col] < new_swir[row][col])
	                 shadow_prob = new_nir[row][col];
                     else
	                 shadow_prob = new_swir[row][col];

                     if (shadow_prob > 200)
	                 shadow_mask[row][col] = 1;
                     else
	                 shadow_mask[row][col] = 0;
                }
            }
        }
        status = ias_misc_free_2d_array((void **)new_nir);
        status = ias_misc_free_2d_array((void **)new_swir);
    }    
    status = ias_misc_free_2d_array((void **)clear_mask);
    status = ias_misc_free_2d_array((void **)clear_land_mask);

    /* Loop through each line in the image */
    for (row = 0; row < nrows; row++)
    {
        for (col = 0; col < ncols; col++)
        {
            /* refine Water mask (no confusion water/cloud) */
            if (water_mask[row][col] == 1 && cloud_mask[row][col] == 0)
                water_mask[row][col] = 1;
            else
	        water_mask[row][col] = 0;

            if (mask[row][col] == 0)
	    {
	        cloud_mask[row][col] = 255;
	        shadow_mask[row][col] = 255;
	    }
        }
    }
    status = ias_misc_free_2d_array((void **)mask);

    return SUCCESS;
    
}

