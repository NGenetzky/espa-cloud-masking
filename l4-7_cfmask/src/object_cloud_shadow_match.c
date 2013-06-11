#include "input.h"
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
#define MAX_CLOUD_TYPE 300000
#define MIN_CLOUD_OBJ 9
#define PI (3.141592653589793238)

static int num_clouds = 0;

/******************************************************************************
MODULE:  viewgeo

PURPOSE: Calculate the geometric parameters needed for the cloud/shadow match 

RETURN: None

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
3/15/2013   Song Guo         Original Development

NOTES: 
******************************************************************************/
void viewgeo
(
   int x_ul,
   int y_ul,
   int x_ur,
   int y_ur,
   int x_ll,
   int y_ll,
   int x_lr,
   int y_lr,
   float *a,
   float *b,
   float *c,
   float *omiga_par,
   float *omiga_per
)
{
  float x_u, x_l, y_u, y_l;
  float k_ulr = 1.0;
  float k_llr = 1.0;
  float k_aver; 

  x_u=(float)(x_ul+x_ur)/2.0;
  x_l=(float)(x_ll+x_lr)/2.0;
  y_u=(float)(y_ul+y_ur)/2.0;
  y_l=(float)(y_ll+y_lr)/2.0;

  if (x_ul != x_ur)
      k_ulr=(float)(y_ul-y_ur)/(float)(x_ul-x_ur); 
                     /* get k of upper left and right points */
  else
      k_ulr = 0.0;
  if (x_ll != x_lr)      
      k_llr=(float)(y_ll-y_lr)/(float)(x_ll-x_lr); 
                    /* get k of lower left and right points */
  else
      k_llr = 0.0; 

  k_aver=(k_ulr+k_llr)/2.0;
  *omiga_par=atan(k_aver); /* get the angle of parallel lines k (in pi) */

  /* AX(j)+BY(i)+C=0 */
  *a=y_u-y_l;
  *b=x_l-x_u;
  *c=y_l*x_u-x_l*y_u;

  *omiga_per=atan((*b)/(*a)); /* get the angle which is perpendicular to the 
                                 trace line */
}


/******************************************************************************
MODULE:  mat_truecloud

PURPOSE:  Calculate shadow pixel locations of a true cloud segment

RETURN: None

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
3/15/2013   Song Guo         Original Development

NOTES: 
******************************************************************************/
/* mat_truecloud function */
void mat_truecloud
(
    int *x,
    int *y,
    int array_length,
    float *h,
    float a,
    float b,
    float c,
    float omiga_par,
    float omiga_per,
    float *x_new,
    float *y_new
)
{
   float dist;
   float dist_par;
   float dist_move;
   float delt_x;
   float delt_y;

   float height=705000.0; /* average Landsat 4,5,&7 height (m) */
   int i;

   for (i = 0; i < array_length; i++)
   {
       dist = (a*(float)x[i]+b*(float)y[i]+c)/(sqrt(a*a+b*b)); 
                /* from the cetral perpendicular (unit: pixel) */
       dist_par = dist/cos(omiga_per-omiga_par);
       dist_move = (dist_par*h[i])/height; /* cloud move distance (m) */
       delt_x = dist_move*cos(omiga_par);
       delt_y = dist_move*sin(omiga_par);

       x_new[i] = x[i] + delt_x; /* new x, j */
       y_new[i] = y[i] + delt_y; /* new y, i */
   }
}

typedef struct cloud_node_t{
    int value;
    int16 row;
    int16 col;
    struct cloud_node_t* parent;
    struct cloud_node_t* child;
}cloud_node;

cloud_node* Find(cloud_node* node) {
    cloud_node* temp;
    /* Find the root */
    cloud_node* root = node;

    if (node->parent == node)
     return node;

    while (root->parent != root) {
        root = root->parent;
    }
    /* Update the parent pointers */
    while (node->parent != node) {
        temp = node->parent;
        node->parent = root;
        node = temp;
    }
    return root;
}

void Union_child(cloud_node* node1, cloud_node* node2) {
    node1->child = node2;
    node2->value = node1->value;     
    while (node2->child != node2)
    {
         node2 = node2->child;
         node2->value = node1->value;
    }
}

cloud_node* Find_child(cloud_node* node) {
    /* Find the root */
    cloud_node* root = node;
    if (node->child == node)
    {
     return node;
    }

    while (root->child != root) {
        root = root->child;
    }

    return root;
}

void find_min(int *array, int nums, int *min, int *index)
{
    int i;

    *min = 0;
    *index = 0;
     for (i = 0; i < nums; i++)
     {
         if (array[i] > 0)
         {
             *min = array[i];
             *index = i;
             break;
         }
     }
     for (i = 0; i < nums; i++)
     {
         if (array[i] > 0 && array[i] < *min)
         {
             *min = array[i];
             *index = i;
         }
     }
}

int label(unsigned char **cloud_mask, int nrows, int ncols, 
          cloud_node **cloud, int *obj_num, int **first_cloud_node)
{ 
    int row, col;
    int array[4];
    int min;
    int index;

    for (row = 0; row < nrows; row++)
    {
     for (col = 0; col <ncols; col++)
     {
      if (cloud_mask[row][col] == 1)
      {
          if (row > 0 && col > 0 && cloud_mask[row-1][col-1] == 1)
              array[0] = cloud[row-1][col-1].value;
          else
              array[0] = 0;
          if ( row > 0 && cloud_mask[row-1][col] == 1)
              array[1] = cloud[row-1][col].value;
          else
              array[1] = 0;
          if (row > 0 && (col < ncols-1) && cloud_mask[row-1][col+1] == 1)
              array[2] = cloud[row-1][col+1].value;
          else
              array[2] = 0;
          if (col > 0 && cloud_mask[row][col-1] == 1)
              array[3] = cloud[row][col-1].value;
          else
              array[3] = 0;
              
          /* The cloud pixel will be labeled as a new cloud if neighboring
             pixels before it are not cloud pixels, otherwise it will be
             labeled as lowest cloud number neighboring it */
          find_min(array, 4, &min, &index);
          if (min == 0)
          {
            num_clouds++;    
            cloud[row][col].value = num_clouds;
            cloud[row][col].row = row;
            cloud[row][col].col = col;
            first_cloud_node[0][num_clouds] = row;
            first_cloud_node[1][num_clouds] = col;
          }
          else
          {
             cloud[row][col].value = min;
             cloud[row][col].row = row;
             cloud[row][col].col = col;
             if (index == 0)
             {
                 cloud[row][col].parent = Find(&cloud[row-1][col-1]);
                 Find_child(&cloud[row-1][col-1])->child = &cloud[row][col];
             }
             else if (index == 1)
             {
                 cloud[row][col].parent = Find(&cloud[row-1][col]);
                 Find_child(&cloud[row-1][col])->child = &cloud[row][col];
             }
             else if (index == 2)
             {
                 cloud[row][col].parent = Find(&cloud[row-1][col+1]);
                 Find_child(&cloud[row-1][col+1])->child = &cloud[row][col];
             }
             else if (index == 3)
             {
                 cloud[row][col].parent = Find(&cloud[row][col-1]);
                 Find_child(&cloud[row][col-1])->child = &cloud[row][col];
             }
             else
                 continue;

             /* If two neighboring pixels are labeled as different cloud
                numbers, the two cloud pixels are relaeled as the same cloud */
             if ((row > 0 && col > 0 && cloud_mask[row-1][col-1] == 1) && 
                 (cloud[row-1][col-1].value != min))
             {
                 if (index == 1)
                 {
                     Union_child(Find_child(&cloud[row-1][col]), 
                                 Find(&cloud[row-1][col-1]));
                     Find(&cloud[row-1][col-1])->parent = 
                            Find(&cloud[row-1][col]);
                 }
                 else if (index == 2)
                 {
                     Union_child(Find_child(&cloud[row-1][col+1]), 
                                 Find(&cloud[row-1][col-1]));
                     Find(&cloud[row-1][col-1])->parent =
                                Find(&cloud[row-1][col+1]);
                 }
                 else if (index == 3)
                 {
                     Union_child(Find_child(&cloud[row][col-1]), 
                                 Find(&cloud[row-1][col-1]));
                     Find(&cloud[row-1][col-1])->parent = 
                                Find(&cloud[row][col-1]);
                 }
                 else
                     continue;
             }
             if ((row > 0 && cloud_mask[row-1][col] == 1) && 
                 (cloud[row-1][col].value != min))
             {
                 if (index == 0)
                 {
                     Union_child(Find_child(&cloud[row-1][col-1]), 
                                 Find(&cloud[row-1][col]));
                     Find(&cloud[row-1][col])->parent = 
                           Find(&cloud[row-1][col-1]);
                                ;
                 }
                 else if (index == 2)
                 {
                     Union_child(Find_child(&cloud[row-1][col+1]), 
                                 Find(&cloud[row-1][col]));
                     Find(&cloud[row-1][col])->parent = 
                               Find(&cloud[row-1][col+1]);
                 }
                 else if (index == 3)
                 {
                     Union_child(Find_child(&cloud[row][col-1]), 
                                 Find(&cloud[row-1][col]));
                     Find(&cloud[row-1][col])->parent = 
                                Find(&cloud[row][col-1]);
                 }
                 else
                     continue;
             }
             if ((row > 0 && (col < ncols-1) && cloud_mask[row-1][col+1] == 1) 
                 && (cloud[row-1][col+1].value != min))
             {
                 if (index == 0)
                 {
                     Union_child(Find_child(&cloud[row-1][col-1]), 
                                 Find(&cloud[row-1][col+1]));
                     Find(&cloud[row-1][col+1])->parent =
                                Find(&cloud[row-1][col-1]);
                 }
                 else if (index == 1)
                 {
                     Union_child(Find_child(&cloud[row-1][col]), 
                                 Find(&cloud[row-1][col+1]));
                     Find(&cloud[row-1][col+1])->parent = 
                                Find(&cloud[row-1][col]);
                 }
                 else if (index == 3)
                 {
                     Union_child(Find_child(&cloud[row][col-1]), 
                                 Find(&cloud[row-1][col+1]));
                     Find(&cloud[row-1][col+1])->parent = 
                                Find(&cloud[row][col-1]);
                 }
                 else
                     continue;
             }
             if ((col > 0 && cloud_mask[row][col-1] == 1) && 
                 (cloud[row][col-1].value != min))
             {
                 if (index == 0)
                 {
                     Union_child(Find_child(&cloud[row-1][col-1]), 
                                 Find(&cloud[row][col-1]));
                     Find(&cloud[row][col-1])->parent = 
                                Find(&cloud[row-1][col-1]);
                 }
                 else if (index == 1)
                 {
                     Union_child(Find_child(&cloud[row-1][col]), 
                                 Find(&cloud[row][col-1]));
                     Find(&cloud[row][col-1])->parent = 
                                Find(&cloud[row-1][col]);
                 }
                 else if (index == 2)
                 {
                     Union_child(Find_child(&cloud[row-1][col+1]), 
                                 Find(&cloud[row][col-1]));
                     Find(&cloud[row][col-1])->parent = 
                                Find(&cloud[row-1][col+1]);
                 }
                 else
                     continue;
             }
          }
      }
     }
    }

    printf("First pass in labeling algorithm done\n");
    /* The second pass labels all cloud pixels according two their root
       parent cloud pixel values */
    for (row = 0; row < nrows; row++)
    {
     for (col = 0; col <ncols; col++)
     {
          if (cloud_mask[row][col] == 1)
          {
              cloud[row][col].value = Find(&cloud[row][col])->value;
              obj_num[cloud[row][col].value]++;
          }
       }
    }

    return 0;
}      

/******************************************************************************
MODULE:  object_cloud_shadow_match

PURPOSE: Identify the final shadow pixels by doing a geometric cloud and shadow
         matching which ends in with the maximum cloud and shadow similarity

RETURN: 0 on success
        -1 on error

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
3/15/2013   Song Guo         Original Development

NOTES: 
******************************************************************************/
int object_cloud_shadow_match
(
    Input_t *input,
    float ptm,
    float t_templ,
    float t_temph,
    int cldpix,
    int sdpix,
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
    int row;
    int col = 0;
    float sun_ele;
    float sun_ele_rad;
    float sun_tazi;
    float sun_tazi_rad;
    int sub_size = 30;
    int status;

    /* Dynamic memory allocation */
    unsigned char **cloud_cal;
    unsigned char **shadow_cal;
    unsigned char **boundary_test;

    cloud_cal = (unsigned char **)ias_misc_allocate_2d_array(input->size.l, 
                 input->size.s, sizeof(unsigned char)); 
    shadow_cal = (unsigned char **)ias_misc_allocate_2d_array(
                 input->size.l, input->size.s, sizeof(unsigned char)); 
    boundary_test = (unsigned char **)ias_misc_allocate_2d_array(
                 input->size.l, input->size.s, sizeof(unsigned char)); 
    if (cloud_cal == NULL || shadow_cal == NULL || boundary_test == NULL)
    {
        sprintf (errstr, "Allocating mask memory");
        ERROR (errstr, "cloud/shadow match");
    }

    /* Read in potential mask ... */
    /* Solar elevation angle */
    sun_ele = 90 - input->meta.sun_zen;
    sun_ele_rad = (PI / 180.0) * sun_ele;
    /* Solar azimuth angle */
    sun_tazi = input->meta.sun_az - 90;
    sun_tazi_rad = (PI / 180.0) * sun_tazi;

    int cloud_counter = 0;
    int boundary_counter = 0;
    float revised_ptm;
    for (row = 0; row < nrows; row++)
    {
        for (col = 0; col < ncols; col++)
        {
            if (cloud_mask[row][col] == 1)
                cloud_counter++;
 
            /* Boundary layer includes both cloud_mask equals 0 and 1 */
            if (cloud_mask[row][col] < 255)     
            {
                boundary_test[row][col] = 1;
                boundary_counter++;
            }
            else
                boundary_test[row][col] = 0;
        }
    }

     /* Revised percent of cloud on the scene after plcloud */
     revised_ptm = (float)cloud_counter / (float)boundary_counter;

     if (verbose)
     {
         printf("cloud_counter, boundary_counter = %d, %d\n", cloud_counter, 
                boundary_counter);
         printf("Revised percent of cloud = %f\n", revised_ptm);
     }

     /* cloud covers more than 90% of the scene
         => no match => rest are definite shadows */
     if (ptm <= 0.1 || revised_ptm >= 0.90)
     {
         for (row = 0; row < nrows; row++)
         {
             for (col = 0; col < ncols; col++)
             {
                 /* No Shadow Match due to too much cloud (>90 percent) */
                 if (cloud_mask[row][col] == 1) 
                     cloud_cal[row][col] = 1;
                 else
                     shadow_cal[row][col] = 1;
             }
         }
     }
     else
     {
         if (verbose)
             printf("Shadow Match in processing\n");

         /* define constants */
         float t_similar=0.30;
         float t_buffer=0.95; /* threshold for matching buffering */
         float max_similar=0.95;/* max similarity threshold */
         int num_cldoj=9; /* minimum matched cloud object (pixels) */
         int num_pix=8; /* number of inward pixes (240m) for cloud base 
                          temperature */
         float a, b, c, omiga_par, omiga_per;

         if (verbose)
         {
             printf("Set cloud similarity = %.3f\n", t_similar);
             printf("Set matching buffer = %.3f\n", t_buffer);
             printf("Shadow match for cloud object >= %d pixels\n", num_cldoj);
         }
         int i_step;
         i_step=rint(2.0*(float)sub_size*tan(sun_ele_rad)); 
                                            /* move 2 pixel at a time */
         /* Get moving direction, the idea is to get the corner rows/cols */
         int x_ul = 0;
         int y_ul = 0;
         int x_lr = 0;
         int y_lr = 0;
         int x_ll = 0;
         int y_ll = 0;
         int x_ur = 0;
         int y_ur = 0;
         for (row = 0; row < nrows; row++)
         {
             for (col = 0; col < ncols; col++)
             {
                 if (boundary_test[row][col] == 1)
                 {
                     y_ul = row;
                     x_ul = col;
                     goto next1;
                 }
             }
        }

        next1:        
        for (col = ncols - 1; col >= 0; col--)
        {
            for (row = 0; row < nrows; row++)
            {
                if (boundary_test[row][col] == 1)
                {
                    y_ur = row;
                    x_ur = col;
                    goto next2;
                }
            }
        }

        next2:
        for (col = 0; col < ncols; col++)
        {
            for (row = nrows - 1; row >= 0; row--)
            {
                if (boundary_test[row][col] == 1)
                {
                    y_ll = row;
                    x_ll = col;
                    goto next3;
                }
            }
        }

        next3:
        for (row = nrows - 1; row >= 0; row--)
        {
            for (col = ncols - 1; col >= 0; col--)
            {
                if (boundary_test[row][col] == 1)
                {
                    y_lr = row;
                    x_lr = col;
                    goto next4;
                }
            }
        }

        next4:
        /* get view angle geometry */
        viewgeo(x_ul,y_ul,x_ur,y_ur,x_ll,y_ll,x_lr,y_lr, &a, &b, &c, 
            &omiga_par, &omiga_per);

        /* Allocate memory for segment cloud portion */
        int *obj_num;
        obj_num = (int *)calloc(MAX_CLOUD_TYPE, sizeof(int));
        cloud_node **cloud;
        cloud = (cloud_node **)ias_misc_allocate_2d_array(nrows, 
               ncols, sizeof(cloud_node)); 
        int **cloud_first_node;
        cloud_first_node = (int **)ias_misc_allocate_2d_array(2, 
               MAX_CLOUD_TYPE, sizeof(int)); 
        if (obj_num == NULL || cloud == NULL || cloud_first_node == NULL)
        {
            sprintf (errstr, "Allocating memory");
            ERROR (errstr, "cloud/shadow match");
            return -1;        
        }

        /* Initialize the cloud nodes */
        for (row = 0; row < nrows; row++)
        {
           for (col = 0; col <ncols; col++)
           {
               cloud[row][col].value = 0;
               cloud[row][col].row = 0;
               cloud[row][col].col = 0;
               cloud[row][col].parent = &cloud[row][col];
               cloud[row][col].child = &cloud[row][col];
           }
        }

        /* Labeling the cloud pixels */
        label(cloud_mask, nrows, ncols, cloud, obj_num, cloud_first_node);

        /* The cloud pixels are not counted as cloud pixels if the total number 
           of cloud pixels is less than 9 within a cloud cluster */
        int num;
        int counter = 0;
        for (num = 1; num <= num_clouds; num++)
        {
           if (obj_num[num] <= MIN_CLOUD_OBJ)
               obj_num[num] = 0;
           else
               counter++;
        }

        if (verbose)
            printf("Num of real clouds = %d\n", counter);

        /* Cloud_cal pixels are cloud_mask pixels with < 9 pixels removed */
        for (row = 0; row < nrows; row++)
        {
           for (col = 0; col <ncols; col++)
           {
              if ((cloud_mask[row][col] == 1) && (boundary_test[row][col] != 0)
                   && (obj_num[cloud[row][col].value] != 0))
                  cloud_cal[row][col] = cloud_mask[row][col];
              else
                  cloud_cal[row][col] = 0;
          }
        }

        /* Need to read out whole image brightness temperature for band 6 */
        int16 **temp;
        temp = (int16 **)ias_misc_allocate_2d_array(input->size.l, 
                 input->size.s, sizeof(int16)); 
        if (!temp)
        {
             sprintf (errstr, "Allocating temp memory");
             ERROR (errstr, "cloud/shadow match");
        }

        /* Read out thermal band in 2d */
        for (row = 0; row < nrows; row++)
        {
	    if (!GetInputThermLine(input, row))
            {
	        sprintf (errstr, "Reading input thermal data for line %d", row);
	        ERROR (errstr,"cloud/shadow match");
	    }
            memcpy(&temp[row][0], &input->therm_buf[0], input->size.s * 
                   sizeof(int16));
        }

        /* Use iteration to get the optimal move distance, Calulate the 
           moving cloud shadow */
        int cloud_type;
        int **xy_type;
        int **tmp_xy_type;
        float **tmp_xys;
        int **orin_xys;
        int16 *temp_obj; 
        int16 temp_obj_max = 0;
        int16 temp_obj_min = 0;
        int index;
        float r_obj;
        float pct_obj;
        float t_obj;
        float rate_elapse=6.5;
        float rate_dlapse=9.8;
        int max_cl_height; /* Max cloud base height (m) */
        int min_cl_height; /* Min cloud base height (m) */
        int max_height;
        int min_height;
        float record_thresh;
        float *record_h;
        int base_h;
        float *h;
        float i_xy;
        int out_all;
        int match_all;
        int total_all;
        float thresh_match;
        int i;

        for (cloud_type = 1; cloud_type <= num_clouds; cloud_type++)
        {
            if (obj_num[cloud_type] == 0)
                continue;
            else
            {
                /* Note: matlab array index starts with 1 and C starts with 0, 
                         array(3,1) in matlab is equal to array[2][0] in C */
                min_cl_height = 200;
                max_cl_height = 12000;
                xy_type = (int **)ias_misc_allocate_2d_array(2, 
                           obj_num[cloud_type], sizeof(int)); 
                tmp_xy_type = (int **)ias_misc_allocate_2d_array(2, 
                     obj_num[cloud_type], sizeof(int)); 
                /* corrected for view angle xys */
                tmp_xys = (float **)ias_misc_allocate_2d_array(2, 
                          obj_num[cloud_type], sizeof(float)); 
                /* record the original xys */
                orin_xys = (int **)ias_misc_allocate_2d_array(2, 
                      obj_num[cloud_type], sizeof(int)); 
                if (xy_type == NULL || tmp_xy_type == NULL || tmp_xys == NULL 
                    || orin_xys == NULL)
                {
                    sprintf (errstr, "Allocating cloud memory");
                    ERROR (errstr, "cloud/shadow match");
                }

                /* Temperature of the cloud object */
                temp_obj = malloc(obj_num[cloud_type] * sizeof(int16));
                if (temp_obj == NULL)
                {
                    sprintf (errstr, "Allocating temp_obj memory");
                    ERROR (errstr, "cloud/shadow match");
                }
     
                temp_obj_max = 0;
                temp_obj_min = 0;
                index = 0;
                cloud_node *node;
                node = &cloud[cloud_first_node[0][cloud_type]]
                       [cloud_first_node[1][cloud_type]];
                while (node->child != node)
                {
                    temp_obj[index] = temp[node->row][node->col];
                    if (temp_obj[index] > temp_obj_max)
                    temp_obj_max = temp_obj[index];
                    if (temp_obj[index] < temp_obj_min)
                    temp_obj_min = temp_obj[index];
                    orin_xys[0][index] = node->col;
                    orin_xys[1][index] = node->row;
                    index++;  
                    node = node->child;         
                }
                temp_obj[index] = temp[node->row][node->col];
                if (temp_obj[index] > temp_obj_max)
                    temp_obj_max = temp_obj[index];
                if (temp_obj[index] < temp_obj_min)
                    temp_obj_min = temp_obj[index];
                orin_xys[0][index] = node->col;
                orin_xys[1][index] = node->row;
                index++;  
                obj_num[cloud_type] = index;

                /* the base temperature for cloud
                   assume object is round r_obj is radium of object */
                r_obj=sqrt((float)obj_num[cloud_type]/PI);
                /* number of inward pixes for correct temperature */
                pct_obj=((r_obj-(float)num_pix)*(r_obj-(float)num_pix)) /
                          (r_obj * r_obj);
                if ((pct_obj-1.0) >= MINSIGMA)
                    pct_obj = 1.0;/* pct of edge pixel should be less than 1 */

                prctile(temp_obj, obj_num[cloud_type],temp_obj_min,
                        temp_obj_max, 100.0 * pct_obj, &t_obj);

                /* refine cloud height range (m) */
                min_height = (int)rint(10.0*(t_templ-t_obj)/rate_dlapse);
                max_height = (int)rint(10.0*(t_temph-t_obj));
                if (min_cl_height < min_height)
                    min_cl_height = min_height;
                if (max_cl_height > max_height)
                    max_cl_height = max_height;
                /* put the edge of the cloud the same value as t_obj */
                for (i = 0; i < obj_num[cloud_type]; i++)
                {
                    if (temp_obj[i] >rint(t_obj))
                         temp_obj[i]=rint(t_obj);
                }

                /* Allocate memory for h and record_h*/
                h = malloc(obj_num[cloud_type] * sizeof(float));
                record_h = calloc(obj_num[cloud_type], sizeof(float));
                if (h == NULL || record_h == NULL)
                {
                    sprintf (errstr, "Allocating h memory");
                    ERROR (errstr, "cloud/shadow match");
                }

                /* initialize height and similarity info */
                record_thresh=0.0;
                for (base_h = min_cl_height; base_h <= max_cl_height; 
                              base_h+=i_step)
                {
                    for (i = 0; i < obj_num[cloud_type]; i++)
                    {
                        h[i]=(10.0*(t_obj-(float)temp_obj[i])) / 
                              rate_elapse+(float)base_h;
                    }
      
                   /* Get the true postion of the cloud
                      calculate cloud DEM with initial base height */
                   mat_truecloud(orin_xys[0], orin_xys[1], obj_num[cloud_type],
                                 h, a, b, c, omiga_par, omiga_per, tmp_xys[0], 
                                 tmp_xys[1]);

                   out_all = 0;
                   match_all = 0;
                   total_all = 0;
                   for (i = 0; i < obj_num[cloud_type]; i++)
                   {
                       i_xy=h[i]/((float)sub_size*tan(sun_ele_rad));
                       if ((input->meta.sun_az - 180.0) < MINSIGMA)
                       {
                            xy_type[1][i] = 
                                rint(tmp_xys[0][i]-i_xy*cos(sun_tazi_rad)); 
                            xy_type[0][i] = 
                                rint(tmp_xys[1][i]-i_xy*sin(sun_tazi_rad)); 
                       }
                       else
                       {
                            xy_type[1][i] =
                                rint(tmp_xys[0][i]+i_xy*cos(sun_tazi_rad)); 
                            xy_type[0][i] = 
                                rint(tmp_xys[1][i]+i_xy*sin(sun_tazi_rad)); 
                       }

                       /* the id that is out of the image */
                       if (xy_type[0][i] < 0 || xy_type[0][i] >= nrows 
                          || xy_type[1][i] < 0 || xy_type[1][i] >= ncols)
                          out_all++;
                       else
                       {
                           if (boundary_test[xy_type[0][i]][xy_type[1][i]] == 0
                               || (cloud[xy_type[0][i]][xy_type[1][i]].value !=
                               cloud_type &&
                               (cloud_mask[xy_type[0][i]][xy_type[1][i]]>0 ||
                               shadow_mask[xy_type[0][i]][xy_type[1][i]] == 1)))
                               match_all++;
                           if (cloud[xy_type[0][i]][xy_type[1][i]].value != 
                               cloud_type)
                               total_all++; 
                        }
                   }
                   match_all += out_all;
                   total_all+=out_all;

                   thresh_match=(float)match_all/(float)total_all;
                   if (((thresh_match - t_buffer*record_thresh) >= MINSIGMA)
                       && (base_h < max_cl_height-i_step) && ((record_thresh
                           - max_similar)<MINSIGMA))
                   {
                       if ((thresh_match - record_thresh) > MINSIGMA)
                       {
                           record_thresh=thresh_match;
                           for (i = 0; i < obj_num[cloud_type]; i++)
                               record_h[i] = h[i];
                       }
                   }
                   else if ((record_thresh - t_similar) > MINSIGMA)
                   {
                       float i_vir;
                       for (i = 0; i < obj_num[cloud_type]; i++)
                       {
                           i_vir = record_h[i] / 
                                   ((float)sub_size*tan(sun_ele_rad));
                           if ((input->meta.sun_az - 180.0) < MINSIGMA)
                           {
                               tmp_xy_type[1][i]=rint(tmp_xys[0][i]-
                                     i_vir*cos(sun_tazi_rad)); 
                               tmp_xy_type[0][i]=rint(tmp_xys[1][i]-
                                     i_vir*sin(sun_tazi_rad)); 
                           }
                           else
                           {
                               tmp_xy_type[1][i]=rint(tmp_xys[0][i]+
                               i_vir*cos(sun_tazi_rad)); 
                               tmp_xy_type[0][i]=rint(tmp_xys[1][i]+
                               i_vir*sin(sun_tazi_rad)); 
                           }

                           /* put data within range */
                           if (tmp_xy_type[0][i]<0)
                               tmp_xy_type[0][i]=0;
                           if (tmp_xy_type[0][i]>=nrows)
                               tmp_xy_type[0][i]=nrows-1;
                           if (tmp_xy_type[1][i]<0)
                               tmp_xy_type[1][i]=0;
                           if (tmp_xy_type[1][i]>=ncols)
                               tmp_xy_type[1][i]=ncols-1;
                           shadow_cal[tmp_xy_type[0][i]][tmp_xy_type[1][i]] = 1;
                       }
                       break;
                   }
                   else
                   {
                       record_thresh=0.0;
                       continue;
                   }
               }
               free(h);
               free(record_h);
               /* Free all the memory */
               status = ias_misc_free_2d_array((void **)xy_type);
               status = ias_misc_free_2d_array((void **)tmp_xys);
               status = ias_misc_free_2d_array((void **)orin_xys);
               free(temp_obj);
            }
       }      
       free(obj_num); 
       status = ias_misc_free_2d_array((void **)temp);
       status = ias_misc_free_2d_array((void **)cloud);

       IplImage* src = cvCreateImage(cvSize(ncols, nrows), IPL_DEPTH_8U, 1);
       IplImage* dst = cvCreateImage(cvSize(ncols, nrows), IPL_DEPTH_8U, 1);
       if (!src || !dst)
       {
            sprintf (errstr, "Creating images\n");
            ERROR (errstr, "cloud/shadow match");
       }
 
       int dilation_type = 1;
       IplConvKernel* element = cvCreateStructuringElementEx( 
              2*cldpix + 1, 2*cldpix + 1, cldpix, cldpix, dilation_type, 0);
 
       for (row = 0; row < nrows; row++)
       {
           for (col = 0; col < ncols; col++)
           {
               src->imageData[row*ncols+col] = cloud_cal[row][col];
           }
       }

       cvDilate(src, dst, element, 1);
       for (row = 0; row < nrows; row++)
       {
           for (col = 0; col < ncols; col++)
           {
               cloud_cal[row][col] = dst->imageData[row*ncols+col];
           }
       }

       element = cvCreateStructuringElementEx( 
              2*sdpix + 1, 2*sdpix + 1, sdpix, sdpix, dilation_type, 0);

       for (row = 0; row < nrows; row++)
       {
           for (col = 0; col < ncols; col++)
           {
               src->imageData[row*ncols+col] = shadow_cal[row][col];
           }
       }

       cvDilate(src, dst, element, 1);
       for (row = 0; row < nrows; row++)
       {
           for (col = 0; col < ncols; col++)
           {
               shadow_cal[row][col] = dst->imageData[row*ncols+col];
           }
       }

       for (row = 0; row < nrows; row++)
       {
           for (col = 0; col < ncols; col++)
           {
               src->imageData[row*ncols+col] = snow_mask[row][col];
           }
        }

        cvDilate(src, dst, element, 1);
        for (row = 0; row < nrows; row++)
        {
            for (col = 0; col < ncols; col++)
            {
                snow_mask[row][col] = dst->imageData[row*ncols+col];
            }
         }

         /* Release image memory */
         cvReleaseImage(&src);
         cvReleaseImage(&dst);
     }

     /* Use cloud mask as the final output mask */  
     int cloud_count = 0;    
     int shadow_count = 0;    
     for (row = 0; row < nrows; row++)
     {
         for (col = 0; col < ncols; col++)
         {
             if (boundary_test[row][col] ==0)
                 final_mask[row][col] = 255;
             else if (cloud_cal[row][col] == 1)
             {
	         final_mask[row][col] = 4;
                 cloud_count++;
             }
             else if (shadow_cal[row][col] == 1)
             {
	         final_mask[row][col] = 2;
                 shadow_count++;
             }
             else if (snow_mask[row][col] == 1)
                 final_mask[row][col] = 3;
             else if (water_mask[row][col] == 1)
	         final_mask[row][col] = 1;
             else 
                 final_mask[row][col] = 0;
         }
      }

      /* Release the memory */
      status = ias_misc_free_2d_array((void **)cloud_cal);
      status = ias_misc_free_2d_array((void **)shadow_cal);
      status = ias_misc_free_2d_array(( void **)boundary_test);

      if (verbose)
      {
          printf("cloud_count, shadow_count, boundary_counter = %d,%d,%d\n",
              cloud_count, shadow_count, boundary_counter);

          /* record cloud and cloud shadow percent; */
          float cloud_shadow_percent;
          cloud_shadow_percent = (float)(cloud_count + shadow_count)
                          / (float)boundary_counter;
          printf("The cloud and shadow percentage is %f\n", cloud_shadow_percent);
      }
  
      return 0;
}
